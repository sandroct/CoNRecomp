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
- C1.1 — Criado <this repository> com .gitignore PRIMEIRO (regra nº 1). Pastas: docs, game, recomp, tools,
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
- C1.11 — .gitignore: adicionados `generated/` e `textures/` antes do commit.
- C1.12 — git init -b main; add -A; commit 665a9b1 = Etapa 1 CONCLUÍDA.

## Etapa 2 — Extração e identificação (2026-10-21)

- E2.1 — ISO confirmada: `C:\PROJETOS\...\ISO E BIOS\Champions of Norrath (USA).iso`, 7,75 GB (hipótese: DVD-DL
  dual-layer; não verificado ainda). BIOS ao lado: scph39001.bin/.mec/.nvm + SCPH30004R.bin.
- E2.2 — Escrito `tools/win/iso_read.ps1`: leitor ISO9660 genérico (PowerShell puro) porque Python não está
  instalado e iso_extract.py precisa dele. MOTIVO: contornar dependência para a Etapa 2; Python segue
  obrigatório para Etapa 4 (build.py generate).
- E2.3 — `iso_read.ps1 -ExtractPath SYSTEM.CNF`: **BOOT2 = cdrom0:\SLUS_205.65;1 — VER = 1.02 — VMODE = NTSC**.
  Hipótese SLUS_205.65 CONFIRMADA por leitura direta do disco.
- E2.4 — ELF extraído: `extracted/SLUS_205.65`, 9.249.659 bytes, SHA-256
  `9A60F13C5C316D619A288821CC9496CC61864106B7872B52EE141CAED3756FF0`.
- E2.5 — Comparação com `your Ghidra analysis of the SLUS-20565 ELF\game.elf`: SHA-256 IDÊNTICO. É o mesmo ELF
  analisado no Ghidra → o mapa `recomp/cn_functions.csv` (14.659 funções) continua válido.
- E2.6 — Entrada do ELF para Etapa 4: entry 0x00100008 (do CSV, ENTRYPOINT 0x00100008, tamanho 576).
- E2.7 — Listagem completa do disco (disc_listing.txt): primeira tentativa vazou por O(n²) num array PS;
  Read-Dir trocado para ArrayList; rodando em background.
- E2.8 — Walker reescrito de forma iterativa (fila + visited) após exit 255 (suspeita: estouro de pilha por
  recursão profunda). Resultado: 674 entradas. IRX em MODULES/ + MODULES2/ + BG/0MOD/: DNAS*.IMG
  (autenticação online), MTAPMAN (multitap), ATAD, CDVDSTM, SIO2MAN/PADMAN, rede (SMAP/INET/DEV9/PPP),
  USB (USBD/USBKB), áudio (SNDDRV/SDRDRV/LGAUD/MODMIDI). Dados: .LMP (376), .TEX (até 48 MB),
  .GOB, .WAD, .VAT, .XML, .CNF. NETGUI2.ELF (5,6 MB) na raiz — hipótese: front-end de rede.
- E2.9 — docs/GAME.md preenchido; commit 73998b2 (Etapa 2).

## Etapa 3 — Análise (iniciada 2026-10-21)
- E3.1 — Python 3.12.10 instalado (winget, autorizado pelo usuário). Path real:
  python (3.10+) (o PATH do shell do agente ainda
  aponta para o stub da Microsoft Store — usar caminho completo por ora).
- E3.2 — recomp/cn.toml criado no formato do fork (ghidra_map_merge=true, auto_stub_by_name=false,
  direct_tail_calls=false, patch_*=false), derivado do SLUS-20565 (sem valores do Urban Reign).
- E3.3 — Mapa validado: sceGsResetGraph,0x002D16E8 presente em cn_functions.csv com o mesmo endereço dos
  stubs do draft → mapa do Ghidra e os 431 stubs apontam para as mesmas funções.
- E3.4 — GameIndex do PCSX2: thirdparty/pcsx2/ é a seleção mínima do GS, sem o frontend/GameIndex.
  Correções específicas do SLUS-20565: PENDENTE (docs/GAME.md) — virão do frontend do PCSX2/repositório
  oficial quando necessário.

## Etapa 5 — link do con.exe (build distribuído com o PC do irmão, 2026-10-22)

- **Metade A + metade B compiladas**: 90.886 `.obj` no disco (parte A nossa + parte B do irmão: 29.146 arquivos,
  0 falhas, toolchain idêntico MSVC 14.44.35207). Fusão pelo `.ninja_log` do irmão (paths absolutos → relativos,
  `tools/merge_ninja_log.py`).
- **Problema**: o ninja descarta o `.ninja_log` quando reexecuta o CMake (re-glob) no início do build —
  a fusão pelo log não sobreviveu. **Decisão**: caminho manual (sem ninja): gerar rsps, `lib.exe` por partes.
- `LNK1248`: um `.lib` não passa de 4 GB → 90.886 objs (~16 GB de código) divididos em **16 partes**
  (`extracted/parts/p0..p15.lib`, ~1,1 GB cada) que vão direto para o link (`tools/make_link.py`).
- Libs auxiliares construídas: `con_game_code_huge.lib`, `ps2_runtime`, `ps2_iop`, `pcsx2_gs`, `pcsx2_gs_host`,
  `pcsx2_fmt`, `rlImGui`, `imgui`, `raylib`, `SDL3`, ffmpeg.
- Ferramentas novas: `tools/merge_ninja_log.py`, `tools/finalize_build.py`, `tools/fusao_check.py`,
  `tools/make_link.py`, `tools/fuse_final.ps1`, `tools/build_control.ps1` (Full/Game/Pause/Resume),
  `tools/watchdog.ps1` (tarefa agendada), `tools/split_targets.py`, `tools/build_subset.py`.
- **Pendente**: terminar as 16 partes (~1h) → rodar o link (`extracted/run_final_link.bat`) → `con.exe`.

## Etapa 4 — Primeira recompilação (iniciada 2026-10-21)
- E4.1 — Ambiente: VS2022 BuildTools (MSVC 14.44), CMake 3.31 + Ninja OK. tools/win/vsenv.cmd ajustado
  para BuildTools (fallback Community/Professional/Enterprise).
- E4.2 — Recompilador do fork compilado do código: `cmake -S runtime -B build/recomp -DURBAN_BUILD_RECOMP=ON`
  (a chave do fork é URBAN_BUILD_RECOMP, não CON_) → build target ps2_recomp:
  `build/recomp/ps2xRecomp/ps2_recomp.exe`, copiado para `tools/ps2_recomp.exe` (gitignored).
  Erros/de-risco anteriores: (a) target inexistente sem -DURBAN_BUILD_RECOMP=ON → corrigido;
  (b) primeiro configure baixou deps via FetchContent (raylib 5.5, SDL3 3.4.10, imgui v1.92.7-docking,
  rlImGui 5_5, rabbitizer, libdwarf, fmt, toml11) ~410 MB — normal.
- E4.3 — `tools/generate.py` escrito (Python): system.cnf → BOOT2 → extrai o ELF com iso_extract.py,
  confere SHA-256 (esperado 9A60F13C…56FF0), estagia `recomp/cn.toml` com caminhos absolutos e roda
  ps2_recomp. Rodando em background (extracted + generated/).
- E4.4 — **Recompilação concluída com sucesso**: 90.898 arquivos .cpp em `generated/` (0,56 GB),
  `register_functions.cpp` = 47,98 MB (/Od). 10.399 avisos "unhandled instruction" — todos em
  regiões de dados/DNAS/strings (ex.: `__sce_dnas2_symbol11078` = blob cifrado), não abortaram nada.
  Commit 14c5869 (generate.py) + documentos.

## Etapa 5 — Primeiro build (iniciada 2026-10-21)
- E5.1 — Infra genérica trazida do referência e adaptada (renomeada): `src/port/log.{cpp,h}` (namespace
  `con::log`, env `CON_TRACE`) e `src/port/sha256.{cpp,h}`. MOTIVO: infra de log/cripto necessária ao
  port; sem valores de jogo. (hang_dump/call_trace/native_profile têm endereços do Urban Reign — só serão
  reescritos no M1 quando necessários.)
- E5.2 — `src/main.cpp` (M1 mínimo): CPU check SSE4.1/AVX2 + `config.ini` (KEY=value → env) +
  `PS2Runtime::initialize/loadELF/run`, sem hooks. Baseado no runner do `runtime/ps2xRuntime`.
- E5.3 — `src/CMakeLists.txt` completo: `con_game_code` + `con_game_code_huge` (>900 KB → MSVC default
  huge-functions, JOB_POOL `con_huge=2`), PCH do runtime, `register_functions.cpp` com `/Od`,
  `pcsx2gs` alvo dependente (CON_PCSX2_GS=ON), `con_vu1_code` opcional para `generated/vu1`.
- E5.4 — Configure OK: `build/port` (Ninja, 90k regras, 55 s de geração).
- E5.5 — **Pasta `src/con` renomeada para `src/port`**: `CON` é nome reservado de dispositivo no Windows
  (como PRN/AUX/NUL) — caminhos com esse componente falham no open() do git/MSYS e somem do
  Get-ChildItem (sintoma: `fatal: could not open ... No such file or directory`). `con.exe` (arquivo)
  não é afetado. Commit 5ca5c99.
- E5.6 — Build do `con` disparado em background (log: extracted/build_con.log). Etapa 5 = aguardar,
  corrigir erros de compilação e compilar `pcsx2gs.dll` do thirdparty.
- E5.7 — 1ª tentativa falhou com 3 erros (1.246 objetos OK): C3859/C1076 (PCH sob pressão de RAM,
  `-j 8` em máquina de 16 GB) e C1002 em `ConstructTextPass` (52 KB, função monstruosa) e
  `ps2___sce_dnas2_symbol11078` (5,6 MB). Correções: `CON_NOOPT_SOURCES` e `CON_HUGE_NOOPT`
  (listas de fontes que compilam com `/Od`), `CON_HUGE_JOBS=1`, build com `-j 4`. Rebuild rodando
  em background (mesmo log).
- E5.8 — 2ª tentativa: 2 falhas novas (PCH em outra função; DNAS mesmo com /Od). 3ª rodada:
  **stubamos os 2 arquivos DNAS** (dados cifrados lidos como código; ver docs/GAME.md), `skip` no
  cn.toml para futuras gerações, e removemos **PCH + `/d2OptimizeHugeFunctions`** do lib normal
  (C4883 automático nas funções enormes = /Od-like; correto, mais lento — velocidade é pós-M1).
  2.993/90.902 objetos OK quando o processo do build foi ceifado (restart do servidor; sem cl/ninja
  vivos). Re-disparado incremental (commit d011c89).

