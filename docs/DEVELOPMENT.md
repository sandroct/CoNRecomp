# DEVELOPMENT — the complete pipeline and the debug cycle
# DESENVOLVIMENTO — o pipeline completo e o ciclo de debug

🇺🇸 Everything a new contributor needs to go from "fresh clone" to "running the game" to "fixing the
next bug", with the exact commands this project uses.
🇧🇷 Tudo que um novo colaborador precisa para ir de "clone novo" a "jogo rodando" a "corrigir o
próximo bug", com os comandos exatos que este projeto usa.

---

## 1. The pipeline / O pipeline

```
game/*.iso            (your own disc — never in the repo)
   │  python tools/generate.py --iso <path>     (reads SYSTEM.CNF, extracts the ELF, checks SHA-256)
   ▼
build/extracted/SLUS_205.65                     (ELF image + boot metadata)
   │  ps2_recomp (tools/ps2_recomp.exe, built from runtime/ps2xRecomp)
   │  config: recomp/cn.toml + recomp/cn_functions.csv (Ghidra map, 14,659 functions)
   ▼
generated/*.cpp                                (90,898 files — never in the repo)
   │  cmake --build build/port --target con    (con_game_code + con_game_code_huge + runtime + pcsx2gs)
   ▼
build/port/bin/con.exe + pcsx2gs.dll           (the Windows port)
   │  run with the ELF image as argv[1], or keep the ISO in game/
   ▼
the game runs; every problem is one /implement → build → run → read log → fix/ cycle
```

## 2. Where things live / Onde cada coisa vive

| path | what |
|---|---|
| `build.py` | everything: menu, generate, build, play, configure, bench |
| `tools/generate.py` | disc → ELF (SHA-256 gate) → recompile |
| `recomp/cn.toml`, `recomp/cn_functions.csv` | recompiler config + Ghidra function map |
| `src/main.cpp` | the port entry: CPU check, `config.ini`, runtime init, ELF load, run |
| `src/port/` | port sources: `log`, `sha256`, `hle_bindings.cpp` (431 stub bindings) |
| `runtime/` | PS2Recomp runtime fork (EE/VU/GS/IOP + offline tools) |
| `thirdparty/pcsx2-gs` | PCSX2 GS built as `pcsx2gs.dll` |
| `docs/` | STATE (compass), PROGRESS (log), GAME (facts), ROADMAP, ONLINE, L10N, ECOSSISTEMA, AUDIT |
| `extracted/`, `generated/`, `build/`, `game/` | user-side artifacts — **gitignored, never committed** |

## 3. Commands you will actually use / Comandos que você vai usar

```bat
rem status of the long build (compact report)
python tools/build_status.py

rem full build of the port (first build: hours on 16 GB of RAM)
tools\win\vsenv.cmd cmake -S . -B build\port -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
tools\win\vsenv.cmd cmake --build build\port --target con -- -j 4

rem only the PCSX2 GS DLL (built from thirdparty/, never copied)
tools\win\vsenv.cmd cmake --build build\port --target pcsx2gs -- -j 4

rem regenerate the game code (needs your ISO; skips nothing — checks the ELF hash)
python tools\generate.py --iso "game\Champions of Norrath (USA).iso"

rem re-generate the HLE bindings after a new function map
python tools\gen_hle_bindings.py
python tools\hle_coverage.py         (does the runtime have a handler for every stub?)

rem run the port (ELF image next to it, or pass the path)
build\port\bin\con.exe build\extracted\SLUS_205.65
```

## 4. Environment variables / Variáveis de ambiente

Set them in `config.ini` (next to `con.exe`) or the environment; the environment wins.

| variable | effect |
|---|---|
| `PS2X_GS_RENDERER` | `pcsx2` (pcsx2gs.dll, default) or `native` |
| `PS2X_GS_SCALE` | internal resolution multiplier (1–8) |
| `PS2X_PNACH` | `.pnach` patch files, `;`-separated |
| `PS2X_MULTITAP` | ports with a multitap (`12` = both) |
| `PS2X_VU1_CAPTURE` | `1` = dump VU1 microcode images for `vu1_recomp` |
| `PS2X_DETHASH` | every N frames, hash guest state (determinism tests) |
| `CON_TRACE` | categories of the port log (see `src/port/log.h`) |
| `CON_ISO` / `CON_JOBS` / `CON_WORKERS` / `CON_DEPS` | disc image path / build jobs / recompiler workers / prebuilt deps folder |

## 5. The debug cycle / O ciclo de debug

1. **Run, capture the log** (`extracted\runtime.log` or the console). The port prints what it binds and
   what is missing.
2. **Read the signals**:
   - `[Syscall TODO]` → a syscall number with no handler (see `runtime/ps2xRuntime/src/lib/ps2_syscalls.cpp`)
   - `unhandled instruction` → usually data read as code (like the DNAS blob) or a decoder gap; check
     the address with the function map first
   - `[hle] ... stubs bound` → the bind count; a drop means the map and the ELF disagree
3. **Locate the guest code**: address → `recomp/cn_functions.csv` (function), then Ghidra/REA for the
   disassembly (Ghidra 12 + ghidra-emotionengine-reloaded; REA with `provider_id: "ghidra"`).
4. **Fix at the right layer**: runtime handler (`runtime/`), game override (`src/port/`, keyed by
   ELF/entry/CRC32 via `game_overrides.h`), or recompilation config (`recomp/cn.toml`). Never mix.
5. **Rebuild only what changed** (Ninja is incremental) and record the cycle in `docs/PROGRESS.md`
   (date, command, result, address, reason) — the log is the project's memory.

## 6. Known environment traps / Armadilhas conhecidas do ambiente

| trap | fix |
|---|---|
| MSVC heap: `C1060`/`C1002` on big generated functions | compile those files `/Od`: `CON_NOOPT_SOURCES` / `CON_NOOPT_PATTERNS` in `src/CMakeLists.txt` (only that file's rule changes) |
| Corrupted `.pdb` after a killed build (`C1051`) | delete the `.pdb`; no `.obj` is lost |
| A folder named `con` anywhere in the path | impossible on Windows (reserved device name); the port sources live in `src/port/` |
| Background build dies with the session | run detached (`extracted/run_build.bat`) and re-run; Ninja resumes |
| Machine with little RAM | `-j 4`, no PCH, no `/d2OptimizeHugeFunctions`, page file 32 GB |
| Antivirus locking new files (git `open()` ENOENT) | an exclusion for the repo folder; retry |

## 7. Searching code instead of reading it / Buscar código em vez de ler

This project keeps the closest reference repositories **indexed with XERJ** (an agent-oriented search
engine, Apache-2.0, installed at `~/bin/xerj/...`, node on `localhost:9200`):

```
xerj search "PS2X_NET_RELAY netplay relay"     # lexical + AST definitions
xerj def "bindAddressHandler"                  # go-to-definition
```

Indexed corpora: `dbz-bt4-recomp` (the online implementation), `conrt` (our `runtime/`), `skill`
(PS2Recomp knowledge base). Query the index **before** opening a big file — it answers with
`file:line` + the contract, saving the context window.

## 8. Testing / Testes

- `bench/` — savestates to measure frames/s (`python build.py bench`).
- Determinism: hash the guest state every N frames (`PS2X_DETHASH`) and compare against PCSX2 or a
  second machine (the pattern the recomp-code ports use for netplay testing).
- Graphics: compare against PCSX2 screenshots/GS dumps; record every difference in
  `docs/DIVERGENCES.md`.

## 8. Legal / Legal

Bring your own legally obtained disc. Nothing here distributes or links game data, BIOS or assets.
The generated code and the ELF are produced locally from your own copy.
