#ifndef PS2_GAME_OVERRIDES_H
#define PS2_GAME_OVERRIDES_H

#include <cstdint>
#include <string>
#include <string_view>

class PS2Runtime;

namespace ps2_game_overrides
{
    using ApplyFn = void (*)(PS2Runtime &runtime);

    struct Descriptor
    {
        const char *name = "";
        const char *elfName = "";
        uint32_t entry = 0;
        uint32_t crc32 = 0;
        ApplyFn apply = nullptr;
    };

    class AutoRegister
    {
    public:
        explicit AutoRegister(const Descriptor &descriptor);
    };

    void registerDescriptor(const Descriptor &descriptor);
    void applyMatching(PS2Runtime &runtime, const std::string &elfPath, uint32_t entry, uint32_t crc32, bool crc32Valid);

    // Runtime helper for game-specific modules:
    // bind an address to an existing syscall/stub handler by name.
    bool bindAddressHandler(PS2Runtime &runtime, uint32_t address, std::string_view handlerName);

    // True quando o endereço tem um handler C++ (stub HLE) no lugar da função
    // recompilada: esses handlers podem não definir o pc (o scheduler dá o
    // "jr $ra" implícito). Funções recompiladas NUNCA entram aqui - o retorno
    // com o pc inalterado é a reentrada legítima de um laço interno.
    bool isHleHandlerAddress(uint32_t address);
}

#define PS2_GAME_OVERRIDE_JOIN2(a, b) a##b
#define PS2_GAME_OVERRIDE_JOIN(a, b) PS2_GAME_OVERRIDE_JOIN2(a, b)

#define PS2_REGISTER_GAME_OVERRIDE(NAME_LITERAL, ELF_NAME_LITERAL, ENTRY_U32, CRC32_U32, APPLY_FN)      \
    namespace                                                                                             \
    {                                                                                                     \
        static const ps2_game_overrides::Descriptor PS2_GAME_OVERRIDE_JOIN(kPs2OverrideDesc_, __LINE__) \
        {                                                                                                 \
            NAME_LITERAL,                                                                                 \
            ELF_NAME_LITERAL,                                                                             \
            static_cast<uint32_t>(ENTRY_U32),                                                             \
            static_cast<uint32_t>(CRC32_U32),                                                             \
            APPLY_FN                                                                                      \
        };                                                                                                \
        static const ps2_game_overrides::AutoRegister PS2_GAME_OVERRIDE_JOIN(kPs2OverrideReg_, __LINE__)\
            (PS2_GAME_OVERRIDE_JOIN(kPs2OverrideDesc_, __LINE__));                                       \
    }

#endif // PS2_GAME_OVERRIDES_H
