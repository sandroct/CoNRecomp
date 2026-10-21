# THIRD-PARTY

Third-party components used by CoNRecomp and their licenses.

| component | what | from | license |
|---|---|---|---|
| `runtime/` | PS2Recomp runtime, recompiler and offline tools (EE/VU/GS/IOP host runtime `ps2xRuntime`, IOP emulation `ps2xIOP`, recompiler `ps2xRecomp`, tools `ps2xTest`: vu1_recomp, gs_replay, vu0_audit), with the port-infrastructure changes kept by the urban-reign-recomp project (see `runtime/README.md`) | [TaylorNAlbarnaz/PS2Recomp](https://github.com/TaylorNAlbarnaz/PS2Recomp), branch `sotc-port`, commit `ac9efa0` (2026-10-01), itself from [ran-j/PS2Recomp](https://github.com/ran-j/PS2Recomp) | GPL-3.0 (`runtime/LICENSE`) |
| `runtime/ps2xRuntime/src/lib/ipu/` | IPU decoder | [Ace Combat 5 static recompilation](https://github.com/sal063/Ace-Combat-5-Static-recompilation) | Apache-2.0 (`docs/licenses/AC5-Recomp-APACHE-2.0.txt`) |
| `thirdparty/pcsx2/` | PCSX2's GS (hardware renderer) and its libraries | [PCSX2/pcsx2](https://github.com/PCSX2/pcsx2) commit `2954ebe` (2026-10-11) | GPL-3.0 (`thirdparty/pcsx2/COPYING.GPLv3`) |
| `thirdparty/pcsx2-gs/` | wrapper building `pcsx2gs.dll` with a C interface (from urban-reign-recomp, GPL-3.0) | this project of the urban-reign-recomp port | GPL-3.0 |
| `thirdparty/directx-headers/` | DirectX-Headers includes | [microsoft/DirectX-Headers](https://github.com/microsoft/DirectX-Headers) commit `461ef51` | MIT |
| `thirdparty/` FetchContent deps at build time | raylib, fmt, elfio, toml11, libdwarf, rabbitizer, SDL3 | their upstream repositories | their own licenses (bundled with each `*-src` download) |
| Ghidra + ghidra-emotionengine-reloaded | external analysis tools used to build `recomp/cn_functions.csv` (static analysis only, not shipped) | Ghidra (NSA), ghidra-emotionengine-reloaded | Apache-2.0 and its own licenses |

- Structure/method inspired by the urban-reign-recomp project (GPL-3.0); only generic components were copied —
  see `docs/COPIED_FROM_REFERENCE.md`.
- Champions of Norrath gameplay code: recompiled from the user's own disc; no game data is in this repository.
