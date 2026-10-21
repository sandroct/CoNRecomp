// CoNRecomp - Champions of Norrath: Realms of EverQuest (USA, SLUS-20565) - static recompilation to Windows.
//
// Milestone M1 skeleton: boot to the game's main loop. The recompiled EE code (generated/) replaces the
// interpreter; the ELF image comes from the user's own disc image (game/*.iso). Everything game-specific is
// turned OFF by default here: no function hooks, no HLE by address yet (M1-M8 add them cycle by cycle,
// documented in docs/PROGRESS.md).

#include "ps2_runtime.h"

#if defined(PS2X_ENABLE_DEBUG_UI)
#include "ps2_debug_panel.h"
#endif

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <intrin.h>
#endif

#if defined(_WIN32)
#define CON_ARCH_NAME "SSE41"
#else
#define CON_ARCH_NAME "native"
#endif

namespace
{
#if defined(_WIN32)
    // The generated code and the runtime need SSE4.1 (PCSX2's minimum). AVX2 only when built with CON_ARCH=AVX2.
    bool checkCpu()
    {
        int regs[4] = {};
        __cpuid(regs, 0);
        const int maxLeaf = regs[0];
        __cpuid(regs, 1);
        const bool sse41 = (regs[2] & (1 << 19)) != 0;
        const bool osAvx = (regs[2] & (1 << 27)) != 0 && (regs[2] & (1 << 28)) != 0 && (_xgetbv(0) & 6u) == 6u;
        bool avx2 = false;
        if (maxLeaf >= 7)
        {
            __cpuidex(regs, 7, 0);
            avx2 = osAvx && (regs[1] & (1 << 5)) != 0;
        }
        const bool needAvx2 = std::string(CON_ARCH_NAME) == "AVX2";
        std::fprintf(stderr, "[cpu] SSE4.1 %s, AVX2 %s; build %s\n", sse41 ? "yes" : "NO", avx2 ? "yes" : "no", CON_ARCH_NAME);
        const char *missing = !sse41 ? "SSE4.1" : (needAvx2 && !avx2 ? "AVX2" : nullptr);
        if (!missing)
            return true;
        wchar_t text[256];
        std::swprintf(text, 256,
                      L"This processor does not support %hs, which this build of Champions of Norrath needs.\n"
                      L"Este processador nao tem %hs, que esta versao de Champions of Norrath precisa.", missing, missing);
        MessageBoxW(nullptr, text, L"Champions of Norrath (Recomp)", MB_OK | MB_ICONERROR);
        return false;
    }
#endif

    void printStackTrace()
    {
#if defined(_WIN32)
        void *frames[48];
        const USHORT count = CaptureStackBackTrace(0, 48, frames, nullptr);
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, nullptr, TRUE);
        alignas(SYMBOL_INFO) char storage[sizeof(SYMBOL_INFO) + 256];
        auto *symbol = reinterpret_cast<SYMBOL_INFO *>(storage);
        for (USHORT i = 0; i < count; ++i)
        {
            std::memset(storage, 0, sizeof(storage));
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = 255;
            DWORD64 displacement = 0;
            const bool haveSymbol = SymFromAddr(process, frames[i], &displacement, symbol) != FALSE;
            std::fprintf(stderr, "[crash]   #%u %s+0x%llx\n", static_cast<unsigned>(i), haveSymbol ? symbol->Name : "?",
                         static_cast<unsigned long long>(displacement));
        }
        std::fflush(stderr);
#endif
    }

    void installCrashReporter()
    {
        std::set_terminate([]()
                           {
                               std::fprintf(stderr, "[crash] std::terminate on thread %lu\n", GetCurrentThreadId());
                               printStackTrace();
                               std::_Exit(3); });
    }

    std::string trimText(const std::string &text)
    {
        const size_t begin = text.find_first_not_of(" \t\r");
        const size_t end = text.find_last_not_of(" \t\r");
        return begin == std::string::npos ? std::string() : text.substr(begin, end - begin + 1);
    }

    // config.ini next to con.exe: "KEY=value" lines become environment variables the runtime reads
    // (an environment variable already set wins). Missing file = written with the template below.
    constexpr const char *kDefaultConfig = R"(# Champions of Norrath (recomp) settings, read when con.exe starts.
# An environment variable with the same effect (e.g. PS2X_GS_SCALE) wins over this file.

[Game]
# Disc image of Champions of Norrath (USA, SLUS-20565). Empty = look for one in the "game" folder.
Iso=

[Video]
InternalResolution=1
StartMaximized=0
StartFullscreen=0
Renderer=pcsx2
)";

    void loadConfigFile(const std::filesystem::path &path)
    {
        if (!std::filesystem::exists(path))
            std::ofstream(path, std::ios::binary) << kDefaultConfig;
        std::ifstream file(path);
        if (!file)
            return;
        std::string line;
        while (std::getline(file, line))
        {
            line = trimText(line);
            if (line.empty() || line[0] == '#' || line[0] == ';')
                continue;
            const size_t equals = line.find('=');
            if (equals == std::string::npos)
                continue;
            const std::string key = trimText(line.substr(0, equals));
            const std::string value = trimText(line.substr(equals + 1));
            if (value.empty() || std::getenv(key.c_str()))
                continue;
            const std::string upperKey = [&key]
            {
                std::string up = key;
                std::transform(up.begin(), up.end(), up.begin(), [](unsigned char c)
                               { return static_cast<char>(std::toupper(c)); });
                return up;
            }();
#if defined(_WIN32)
            _putenv_s(upperKey.c_str(), value.c_str());
#else
            setenv(upperKey.c_str(), value.c_str(), 0);
#endif
        }
    }

    std::string normalizeGameId(const std::string &elfName)
    {
        std::string result = elfName;
        for (char &c : result)
        {
            if (c == '_')
                c = '-';
            if (c == '.')
                c = '-';
        }
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c)
                       { return static_cast<char>(std::toupper(c)); });
        return result;
    }
}

int main(int argc, char *argv[])
{
#if defined(_WIN32)
    installCrashReporter();
    if (!checkCpu())
        return 1;
#endif

    try
    {
        // Settings live next to the executable (the "game" and "bench" folders of a release).
        const std::filesystem::path exeDir = std::filesystem::path(argv[0]).parent_path();
        loadConfigFile(exeDir / "config.ini");

        // The guest ELF: <argv[1]> (path or a game/*.iso disc image) or the game folder.
        std::filesystem::path elfPath;
        std::vector<std::filesystem::path> arguments(argv + 1, argv + argc);
        if (!arguments.empty())
        {
            const std::filesystem::path given = arguments.front();
            if (given.extension() == ".iso")
            {
                std::fprintf(stderr, "[disc] %s: extracting SLUS_205.65 to build/extracted/ first "
                                     "(python tools/generate.py does it); pass the ELF directly for now\n",
                             given.string().c_str());
                return 2;
            }
            elfPath = std::filesystem::absolute(given);
        }
        else
        {
            for (const char *candidate : {"build/extracted/SLUS_205.65", "extracted/SLUS_205.65", "SLUS_205.65"})
            {
                if (std::filesystem::exists(candidate))
                {
                    elfPath = std::filesystem::absolute(candidate);
                    break;
                }
            }
        }
        if (elfPath.empty() || !std::filesystem::exists(elfPath))
        {
            std::fprintf(stderr, "[main] no guest ELF: pass it as argv[1], or extract one with "
                                 "python tools/generate.py --iso <disc.iso>\n");
            return 2;
        }

        const std::string windowTitle = "CoNRecomp | " + elfPath.filename().string();
        PS2Runtime runtime;
#if defined(PS2X_ENABLE_DEBUG_UI)
        PS2DebugPanel debugPanel;
        runtime.setDebugUiCallbacks(
            [](PS2Runtime &rt, void *userData)
            {
                (void)rt;
                static_cast<PS2DebugPanel *>(userData)->initialize();
            },
            [](PS2Runtime &rt, void *userData)
            {
                static_cast<PS2DebugPanel *>(userData)->draw(rt);
            },
            [](PS2Runtime &rt, void *userData)
            {
                (void)rt;
                static_cast<PS2DebugPanel *>(userData)->shutdown();
            },
            &debugPanel);
#endif
        if (!runtime.initialize(windowTitle.c_str()))
        {
            std::cerr << "Failed to initialize PS2 runtime" << std::endl;
            return 1;
        }
        if (!runtime.loadELF(elfPath.string()))
        {
            std::cerr << "Failed to load ELF file: " << elfPath.string() << std::endl;
            return 1;
        }

        runtime.run();

        std::cout.flush();
        std::cerr.flush();
        std::_Exit(0);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[main] fatal exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[main] fatal exception: unknown" << std::endl;
    }

    std::cout.flush();
    std::cerr.flush();
    std::_Exit(1);
}
