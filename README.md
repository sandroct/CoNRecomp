# CoNRecomp

A static recompilation port of **Champions of Norrath: Realms of EverQuest** (PS2, USA, SLUS-20565) to Windows,
built with the [PS2Recomp](https://github.com/ran-j/PS2Recomp) method.

- The game's MIPS R5900EE code is recompiled to native C++ ahead of time (`generated/`, never in the repo).
- The original IOP kernel modules the disc ships (`IRX` files) run under the emulated IOP (`runtime/ps2xIOP`).
- Graphics go through PCSX2's GS (`thirdparty/pcsx2-gs` built as `pcsx2gs.dll`).
- VU1 microprograms can be compiled ahead of time with `vu1_recomp` (`runtime/ps2xTest`).

The structure and method follow the urban-reign-recomp project (GPL-3.0); every game-driven value
(function map, recompilation config, HLE bindings) is derived from analysis of Champions of Norrath itself.

## Status

Work in progress. See `docs/STATE.md` (current stage), `docs/PROGRESS.md` (log) and `docs/AUDIT.md` (audit).

## Building

1. Copy **your own** Champions of Norrath (USA) disc image into `game/`.
2. `python build.py` — a numbered menu; `generate` recompiles game code, `build` compiles `con.exe` (first build
   takes over an hour), `play` runs it.

Requirements: 64-bit Windows 10/11, Visual Studio 2022 with C++ (CMake and Ninja included), Python 3.10+, a GPU
with DX11, ~20 GB of disk.

## License

This project is GPL-3.0 (see `LICENSE`). Third-party components and their licenses are listed in
`THIRD-PARTY.md`. Nothing in this repository (or in its releases) is game data: the disc image, BIOS, IRX modules,
assets and the generated code are provided by the user and are never committed or distributed.
