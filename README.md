# CoNRecomp

**Champions of Norrath: Realms of EverQuest** (PlayStation 2, USA — **SLUS-20565**) as a native Windows
program, via ahead-of-time static recompilation.

> 🇧🇷 **Português (Brasil):** veja [README.pt-BR.md](README.pt-BR.md).

## What this is

- The game's MIPS R5900EE code is translated ahead of time into C++ (`generated/` — 90,898 files, never
  in this repository) and compiled into a single Windows executable (`con.exe`).
- The original IOP kernel modules the disc ships (`IRX`) run under the emulated IOP (`runtime/ps2xIOP`).
- Graphics go through **PCSX2's GS** (built from `thirdparty/pcsx2-gs` as `pcsx2gs.dll`).
- VU1 microprograms can be compiled ahead of time (`vu1_recomp`, `runtime/ps2xTest`).
- The method and structure follow [urban-reign-recomp](https://github.com/) (GPL-3.0); **every
  game-driven value** (function map, recompilation config, HLE bindings) is derived from analysis of
  *Champions of Norrath* itself — nothing game-specific is copied from another port.

## Status

Work in progress — this is engineering in public, not a release.

| stage | state |
|---|---|
| 0–4 — audit, project setup, disc/ELF identification (SHA-256 verified), analysis, **recompilation of the game code (90,898 functions)** | ✅ done |
| 5 — first build of `con.exe` (currently compiling, ~15% of 90,902 objects) | 🔄 in progress |
| 6+ — runtime milestones M1 (boot) … M8 (autotest vs. PCSX2) | ⬜ planned |

Everything is documented: [`docs/STATE.md`](docs/STATE.md) (current stage),
[`docs/PROGRESS.md`](docs/PROGRESS.md) (full log), [`docs/GAME.md`](docs/GAME.md) (game facts),
[`docs/AUDIT.md`](docs/AUDIT.md) (environment audit).

## Building

1. Put **your own** Champions of Norrath (USA) disc image in `game/`.
2. `python build.py` — numbered menu: `generate` recompiles the game code, `build` compiles `con.exe`
   (the first build takes hours on a 16 GB machine), `play` runs it.

Requirements: 64-bit Windows 10/11, Visual Studio 2022 with C++ (CMake/Ninja included), Python 3.10+,
a DX11 GPU, ~30 GB of disk.

## Roadmap highlights

- **M1** boot to the main menu (HLE by address is ready: 431/431 SDK stubs bound to runtime handlers).
- **Local multiplayer** (multitap, 2–4 players) as M6.
- **LAN/online** plan in [`docs/ONLINE.md`](docs/ONLINE.md) (home network play with a private server —
  the community has already proven the path with PCSX2 + *Champions Reborn Server*).
- **Brazilian Portuguese translation** plan in [`docs/L10N.md`](docs/L10N.md) (the engine already
  supports `pt` in its language table; the disc ships EN/FR/DE/SP text packs).

## Credits

- Runtime/recompiler family: [ran-j/PS2Recomp](https://github.com/ran-j/PS2Recomp) and the
  `sotc-port` fork (TaylorNAlbarnaz) — GPL-3.0.
- GS: [PCSX2](https://pcsx2.net/) — GPL-3.0. IPU decoder: Ace Combat 5 static recompilation — Apache-2.0.
- Analysis: Ghidra + [ghidra-emotionengine-reloaded](https://github.com/chaoticgd/ghidra-emotionengine-reloaded).
- Method reference: urban-reign-recomp (GPL-3.0). Community resources mapped in
  [`docs/ECOSYSTEM.md`](docs/ECOSYSTEM.md) (recomp-code ports, Open-Champions, Champions Reborn Server,
  DNAS bypass patches, the Snowblind archive tooling).

## License

GPL-3.0 ([`LICENSE`](LICENSE)). Third-party components and their licenses: [`THIRD-PARTY.md`](THIRD-PARTY.md).

**No game data is in this repository.** The disc image, BIOS, IRX modules, assets and the generated code
belong to the user and are never committed or distributed. Bring your own legally obtained disc.
