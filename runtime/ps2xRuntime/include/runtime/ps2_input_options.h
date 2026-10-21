#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ps2_host_input
{
    constexpr uint32_t kInvertRightX = 1u;
    constexpr uint32_t kInvertRightY = 2u;

    struct MouseCameraInput
    {
        float x = 0.0f;
        float y = 0.0f;
        bool active = false;
    };

    void setRightStickInvertDefault(uint32_t axes);
    void setMouseCameraEnabled(bool enabled);
    void setMouseStickEnabled(bool enabled);
    [[nodiscard]] MouseCameraInput consumeMouseCameraInput();

    // Controller pad (0-based) settings when controls.ini has no [Pad<pad+1>] section: PCSX2's [PadN] keys in file
    // order, a key repeated once per binding. urban.exe writes controls.ini with them.
    [[nodiscard]] std::vector<std::pair<std::string, std::string>> defaultPadSettings(uint32_t pad);
}
