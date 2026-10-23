#include "game_overrides.h"
#include "ps2_runtime.h"
#include "ps2_runtime_calls.h"
#include "ps2_stubs.h"
#include "ps2_syscalls.h"
#include "ps2_log.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    std::mutex &registryMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    std::vector<ps2_game_overrides::Descriptor> &descriptorRegistry()
    {
        static std::vector<ps2_game_overrides::Descriptor> registry;
        return registry;
    }

    // Endereços cujo handler C++ (stub HLE) substituiu a função recompilada.
    std::unordered_set<uint32_t> &hleHandlerAddresses()
    {
        static std::unordered_set<uint32_t> addresses;
        return addresses;
    }

    // Handler original por endereço (o trampoline genérico consulta isso) e a
    // chave de log do PS2X_TRACE_HLE.
    std::unordered_map<uint32_t, PS2Runtime::RecompiledFunction> &hleRawHandlers()
    {
        static std::unordered_map<uint32_t, PS2Runtime::RecompiledFunction> handlers;
        return handlers;
    }

    bool hleTraceEnabled()
    {
        static const bool enabled = []() { const char *v = std::getenv("PS2X_TRACE_HLE"); return v && *v; }();
        return enabled;
    }

    // Trampoline de todos os handlers HLE: loga a chamada e o retorno
    // (PS2X_TRACE_HLE=1) e dá o "jr $ra" implícito quando o handler não
    // define o pc - sem isso qualquer despacho direto (tail-jump, scheduler)
    // re-despacharia o mesmo endereço para sempre (travamento de boot: o
    // memset do sceSifInitRebootNotify, 10M chamadas/s, 0 ciclos).
    void hleTrampoline(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        const uint32_t entry = ctx->pc;
        const auto it = hleRawHandlers().find(entry);
        if (it == hleRawHandlers().end())
        {
            ctx->pc = getRegU32(ctx, 31);
            return;
        }
        if (hleTraceEnabled())
        {
            std::fprintf(stderr, "[hle-call] 0x%08x a0=%08x a1=%08x a2=%08x a3=%08x\n", entry,
                         getRegU32(ctx, 4), getRegU32(ctx, 5), getRegU32(ctx, 6), getRegU32(ctx, 7));
        }
        it->second(rdram, ctx, runtime);
        if (hleTraceEnabled())
        {
            std::fprintf(stderr, "[hle-ret ] 0x%08x v0=%08x pc=%08x\n", entry, getRegU32(ctx, 2), ctx->pc);
        }
        if (ctx->pc == entry)
        {
            ctx->pc = getRegU32(ctx, 31);
        }
    }

    bool equalsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (size_t i = 0; i < lhs.size(); ++i)
        {
            const auto l = static_cast<unsigned char>(lhs[i]);
            const auto r = static_cast<unsigned char>(rhs[i]);
            if (std::tolower(l) != std::tolower(r))
            {
                return false;
            }
        }

        return true;
    }

    std::string basenameFromPath(const std::string &path)
    {
        std::error_code ec;
        const std::filesystem::path fsPath(path);
        const std::filesystem::path leaf = fsPath.filename();
        if (leaf.empty())
        {
            return path;
        }
        return leaf.string();
    }

    std::optional<PS2Runtime::RecompiledFunction> resolveHandlerByName(std::string_view handlerName)
    {
        const std::string_view resolvedSyscall = ps2_runtime_calls::resolveSyscallName(handlerName);
        if (!resolvedSyscall.empty())
        {
#define PS2_RESOLVE_SYSCALL(name)                   \
    if (resolvedSyscall == std::string_view{#name}) \
    {                                               \
        return &ps2_syscalls::name;                 \
    }
            PS2_SYSCALL_LIST(PS2_RESOLVE_SYSCALL)
#undef PS2_RESOLVE_SYSCALL
        }

        const std::string_view resolvedStub = ps2_runtime_calls::resolveStubName(handlerName);
        if (!resolvedStub.empty())
        {
#define PS2_RESOLVE_STUB(name)                   \
    if (resolvedStub == std::string_view{#name}) \
    {                                            \
        return &ps2_stubs::name;                 \
    }
            PS2_STUB_LIST(PS2_RESOLVE_STUB)
#undef PS2_RESOLVE_STUB
        }

        return std::nullopt;
    }
}
namespace ps2_game_overrides
{
    AutoRegister::AutoRegister(const Descriptor &descriptor)
    {
        registerDescriptor(descriptor);
    }

    void registerDescriptor(const Descriptor &descriptor)
    {
        if (!descriptor.apply)
        {
            std::cerr << "[game_overrides] ignoring descriptor with null apply callback." << std::endl;
            return;
        }

        std::lock_guard<std::mutex> lock(registryMutex());
        descriptorRegistry().push_back(descriptor);
    }

    bool bindAddressHandler(PS2Runtime &runtime, uint32_t address, std::string_view handlerName)
    {
        const auto resolved = resolveHandlerByName(handlerName);
        if (!resolved.has_value())
        {
            std::cerr << "[game_overrides] unresolved handler '" << handlerName
                      << "' for address 0x" << std::hex << address << std::dec << std::endl;
            return false;
        }

        if (!runtime.replaceFunction(address, hleTrampoline))
        {
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(registryMutex());
            hleHandlerAddresses().insert(address);
            hleRawHandlers()[address] = resolved.value();
        }
        return true;
    }

    bool isHleHandlerAddress(uint32_t address)
    {
        std::lock_guard<std::mutex> lock(registryMutex());
        return hleHandlerAddresses().find(address) != hleHandlerAddresses().end();
    }

    void applyMatching(PS2Runtime &runtime,
                       const std::string &elfPath,
                       uint32_t entry,
                       uint32_t fileCrc32,
                       bool fileCrcValid)
    {

        std::vector<Descriptor> descriptors;
        {
            std::lock_guard<std::mutex> lock(registryMutex());
            descriptors = descriptorRegistry();
        }

        if (descriptors.empty())
        {
            return;
        }

        const std::string elfName = basenameFromPath(elfPath);
        size_t appliedCount = 0;
        for (const Descriptor &descriptor : descriptors)
        {
            if (!descriptor.apply)
            {
                continue;
            }

            if (descriptor.elfName && descriptor.elfName[0] != '\0')
            {
                if (!equalsIgnoreCaseAscii(descriptor.elfName, elfName))
                {
                    continue;
                }
            }

            if (descriptor.entry != 0u && descriptor.entry != entry)
            {
                continue;
            }

            if (descriptor.crc32 != 0u)
            {
                if (!fileCrcValid || fileCrc32 != descriptor.crc32)
                {
                    continue;
                }
            }

            const char *name = (descriptor.name && descriptor.name[0] != '\0')
                                   ? descriptor.name
                                   : "unnamed";
            RUNTIME_LOG("[game_overrides] applying '" << name << "'");
            descriptor.apply(runtime);
            ++appliedCount;
        }

        if (appliedCount > 0)
        {
            RUNTIME_LOG("[game_overrides] applied " << appliedCount << " matching override(s).");
        }
    }
}
