# CoNRecomp — PROGRESS

Estilo do referência: data, comando/result, endereço, motivo da mudança.

## 2026-10-21

- AUD 0.1 — Lido runtime/README.md do urban-reign-recomp-main. Result: runtime = fork PS2Recomp
  (TaylorNAlbarnaz sotc-port, commit ac9efa0 2026-10-01) com ps2xRuntime (EE/VU/GS/IOP), ps2xIOP, ps2xRecomp,
  ps2xTest (vu1_recomp, gs_replay, vu0_audit); ipu/ de AC5-Recomp (Apache-2.0). Hipótese de runtime CONFIRMADA.
- AUD 0.2 — Inventário do referência: raiz (build.py, CMakeLists.txt, LICENSE, .gitignore, .gitattributes,
  .recomp.json, README), docs/ (licenses/releases/status, ADVANCED, ENV, GAME, GS_PCSX2, PROGRESS, ROADMAP),
  recomp/, runtime/, src/ (res, urban 29 arq), thirdparty/ (directx-headers, pcsx2, pcsx2-gs, README), tools/
  (16 .py limp + 7 .py urban-espec + autotest/3 + ghidra/1 + status/1 + win/4).
- AUD 0.3 — Grep "(?i)urban" em tools/*.py e subpastas: 14 arquivos com conteúdo urban-específico identificado
  e EXCLUÍDO da cópia (listado em docs/STATE.md).
- C1.1 — Criado C:\recomp\CoNRecomp com .gitignore PRIMEIRO (regra nº 1). Pastas: docs, game, recomp, tools,
  src\con, src\res, extracted, bench.
- C1.2 — copiar runtime/ → runtime/: 299 arq, 3,93 MB (robocopy /E). MOTIVO: runtime genérico do PS2Recomp.
- C1.3 — copiar thirdparty/ → thirdparty/: 369 arq, 13,13 MB. MOTIVO: GS do PCSX2 a ser compilada daqui
  (pcsx2gs.dll NÃO copiada do Urban Reign).
- C1.4 — copiar 16 tools/.py limpos + autotest/frame_stats.py + win/vsenv.cmd. MOTIVO: genéricos.
- C1.5 — adaptar CMakeLists.txt: UrbanReignPC→CoNRecomp, URBAN_*→CON_*, urban→con; mods/modsdk removidos.
- C1.6 — criar src/CMakeLists.txt placeholder (completo no M1).
- C1.7 — copiar .gitattributes, LICENSE (GPL-3.0), docs/licenses/ (AC5-Recomp-APACHE-2.0.txt).
- C1.8 — copiar função-mapa do CoN: recomp/cn_functions.csv (do ExportPS2Functions.java; header
  Name,Start,End,Size; ENTRYPOINT 0x00100008) e recomp/cn_config_ghidra_draft.toml (draft, formato ran-j).
- C1.9 — adaptar build.py (con.exe, CON_JOBS, CON_DEPS, CON_BUILD_RECOMP; launcher/release removidos).
- C1.10 — criar docs/STATE.md, docs/PROGRESS.md (este).
