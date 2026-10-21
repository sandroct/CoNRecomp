# COPIED FROM REFERENCE (urban-reign-recomp, GPL-3.0)

Everything below was copied from the urban-reign-recomp source tree (structure and method of reference) and is
GPL-3.0 unless noted otherwise. **Only generic components were copied**: no hashes, no `urban.toml`, no function
map, no hooks, patches or Urban Reign names derived values.

| copied | from (reference) | license | what was kept |
|---|---|---|---|
| `runtime/**` (299 files) | `runtime/` | GPL-3.0 (runtime/LICENSE); `ps2xRuntime/src/lib/ipu/` Apache-2.0 | unchanged |
| `thirdparty/directx-headers/**` | `thirdparty/directx-headers/` | MIT | unchanged |
| `thirdparty/pcsx2/**` | `thirdparty/pcsx2/` | GPL-3.0 + per-library | unchanged |
| `thirdparty/pcsx2-gs/**` | `thirdparty/pcsx2-gs/` | GPL-3.0 | unchanged (wrapper; the DLL is compiled here, never copied) |
| `thirdparty/README.md` | same | project text | unchanged |
| `tools/{fb_check,frame_scan,frame_waits,gif_dump,gs_vram,image_cadence,irx_info,iso_extract,mc_check,pacing_summary,pine,present_pacing,present_stats,thread_usage,vu1_trace,vu_disasm}.py` | `tools/` | project text (GPL) | unchanged; game-driven parameters stay empty until Etapas 2–4 supply CoN values |
| `tools/autotest/frame_stats.py` | `tools/autotest/` | project text (GPL) | unchanged |
| `tools/win/vsenv.cmd` | `tools/win/` | project text (GPL) | unchanged |
| `docs/licenses/AC5-Recomp-APACHE-2.0.txt` | `docs/licenses/` | Apache-2.0 | unchanged |
| `.gitattributes` | root | project text | unchanged |

## Adapted (method kept, game-driven values replaced)

| file | base (reference) | changes |
|---|---|---|
| `CMakeLists.txt` | `CMakeLists.txt` | `UrbanReignPC`→`CoNRecomp`, `URBAN_*`→`CON_*`, `urban`→`con`; mods/modsdk targets removed (no mods yet) |
| `build.py` | `build.py` | `urban.exe`→`con.exe`, `URBAN_*`→`CON_*`; launcher/release removed (tools were urban-specific; release.py will be adapted later) |
| `src/CMakeLists.txt` | (placeholder) | full version arrives at milestone M1 with the port host sources |

## Own files (not from the reference)

`recomp/cn_functions.csv`, `recomp/cn_config_ghidra_draft.toml` (from our Ghidra analysis of the Champions of
Norrath ELF via `ExportPS2Functions.java`), `.gitignore`, `README.md`, `THIRD-PARTY.md`, `docs/*` (this stage).

## Excluded on purpose (Urban Reign-specific; rewritten/adapted later if needed)

`tools/{find_sdk_functions,generate,ghidra_functions,release,frame_times,task_list,vif_chain}.py`,
`tools/autotest/{bench_state,drive}.py`, `tools/ghidra/DecompileAt.java`, `tools/status/generate.py`,
`tools/win/capture_*.ps1`, `src/urban/**`, `src/res/urban.rc`, `mods/`, `modsdk/`, `launcher/`,
`docs/{GAME,PROGRESS,ROADMAP,ADVANCED,GS_PCSX2,ENV}.md`, `docs/releases/`, `docs/status/`, `.recomp.json`,
`recomp/urban.toml`, `recomp/urban_functions.csv`.

(`iso_extract.py` and the other clean tools were kept because they operate on any disc image; they were verified
to contain no Urban Reign literal strings.)
