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
- E2.5 — Comparação com `C:\Users\Alessandro\Decomp\slus-20565\game.elf`: SHA-256 IDÊNTICO. É o mesmo ELF
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
  C:\Users\Alessandro\AppData\Local\Programs\Python\Python312\python.exe (o PATH do shell do agente ainda
  aponta para o stub da Microsoft Store — usar caminho completo por ora).
- E3.2 — recomp/cn.toml criado no formato do fork (ghidra_map_merge=true, auto_stub_by_name=false,
  direct_tail_calls=false, patch_*=false), derivado do SLUS-20565 (sem valores do Urban Reign).
- E3.3 — Mapa validado: sceGsResetGraph,0x002D16E8 presente em cn_functions.csv com o mesmo endereço dos
  stubs do draft → mapa do Ghidra e os 431 stubs apontam para as mesmas funções.
- E3.4 — GameIndex do PCSX2: thirdparty/pcsx2/ é a seleção mínima do GS, sem o frontend/GameIndex.
  Correções específicas do SLUS-20565: PENDENTE (docs/GAME.md) — virão do frontend do PCSX2/repositório
  oficial quando necessário.

