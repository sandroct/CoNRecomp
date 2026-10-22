# ROADMAP — milestones M1 to M8

🇺🇸 This is the public roadmap: each milestone is a *implement → build → run → read log → fix* cycle,
recorded in `docs/PROGRESS.md`. Acceptance criteria are concrete so a contributor knows when a
milestone is really done.
🇧🇷 Roteiro público: cada marco é um ciclo *implementar → compilar → rodar → ler log → corrigir*,
registrado no `docs/PROGRESS.md`. Os critérios de aceitação são concretos.

## Current position / Posição atual

- ✅ **Stages 0–4 done**: audit, project, identification (ELF SHA-256 `9A60F13C…` verified),
  analysis (function map 14,659 functions), **recompilation (90,898 `.cpp`)**.
- 🔄 **Stage 5**: first build of `con.exe` in progress.
- ⬜ **Stages 6+**: the milestones below.

---

## M1 — Boot to the main loop / Boot até o menu principal

**What / O que**: `con.exe` initializes the runtime, loads the ELF image, runs the entry point, and the
game reaches its main loop (title/menu). Missing SDK functions appear in the log.
**Depends on / Depende de**: Stage 5 (build) + `src/port/hle_bindings.cpp` (431/431 stubs bound).
**Acceptance / Aceitação**:
- runtime log shows `[hle] Champions of Norrath: 431/431 SDK stubs bound by address`
- no crash before the first guest frame; the game's first screen/menu is reached (screenshot in the issue)
**Known risks / Riscos conhecidos**: 430 untracked stubs (pointer-only functions) may be called; triage
handlers (`ret0/ret1/reta0` pattern) unblock checks; VU1 interpreted until M4.

## M2 — IOP, disc, input, memory card, network stub

**What**: the disc `IRX` modules run emulated; the game reads its data files (`cdrom0:` paths —
see `docs/GAME.md`); pad works; memory card visible; the online path is reached but stubbed (no server).
**Acceptance**: character creation screen works with keyboard/controller; IOP RPC log shows ioman/fileio
serving `.LMP`/`.TEX`/`.VAT` reads; network UI appears and fails gracefully offline.

## M3 — Graphics (PCSX2 GS + GameIndex)

**What**: frames render through `pcsx2gs.dll`; PCSX2 GameIndex corrections for serial `BASLUS-20565`
(progressive scan, CRC-dependent fixes) are applied through the port's own override mechanism
(`game_overrides.h`).
**Acceptance**: menus render; in-game scene renders at 60 fps with correct aspect; comparison against
PCSX2 (GS dump / screenshots) in the PR.

## M4 — Audio + VU1 compiled ahead of time

**What**: SPU2/libsdr path works (the `SNDDRV`/`SDRDRV` modules); VU1 microprograms captured with
`PS2X_VU1_CAPTURE` and compiled with `vu1_recomp` into `generated/vu1` (the skinning/world programs
`EXPL_vu1*` identified in the ELF strings).
**Acceptance**: music and SFX audible; VU1 programs compiled (not interpreted) with state-hash
comparison against the interpreter for determinism.

## M5 — Videos (IPU)

**What**: FMV/`.PSS`-style video through the emulated IPU (or the FFmpeg HLE path, `CON_MPEG_HLE=1`).
**Acceptance**: any intro/story video plays (or is documented as absent on this disc — evidence:
no video files found in the ISO listing).

## M6 — Input: 1 player + multitap (4)

**What**: pad bindings via `controls.ini` (PCSX2 format); multitap on the port the game opens
(`sceMtapPortOpen`), 2–4 human players locally.
**Acceptance**: two controllers drive two players in co-op; `PS2X_MULTITAP` documented.

## M7 — Save/load + stage generation determinism

**What**: memory card save/load (`icon.sys`, `DATA2/SAV`-style files), RNG/seed of stage generation
matched to PCSX2 (determinism hashes: `PS2X_DETHASH` pattern from the recomp-code ports).
**Acceptance**: same inputs → same generated stage on both the port and PCSX2 (hash compare in CI or
in the PR description).

## M8 — Autotest + comparison with PCSX2

**What**: scripts that drive the menus (adapted from `tools/autotest/`), capture baseline
screenshots/logs from PCSX2, and compare automatically; `docs/DIVERGENCES.md` records every difference
with evidence.
**Acceptance**: the test runs unattended and produces a pass/fail report; every divergence has an issue.

---

## Stretch goals (after the milestones)

- **LAN play with a friend** (`docs/ONLINE.md`): SMAP→host UDP bridge (port the `PS2X_NET_*` pattern
  from recomp-code/dbz-bt4-recomp), DNS override (`con.soegames.net` → host IP), DNAS bypass (community
  patch values, applied locally), Champions Reborn Server on the host.
- **Brazilian Portuguese translation** (`docs/L10N.md`): extract `GTEXT_`/`SEL_` packs, translate,
  repack as `pt`, menu entry — keeping EN/FR/DE/SP untouched.
