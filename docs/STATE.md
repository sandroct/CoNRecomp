# CoNRecomp — STATE

Data: 2026-10-21

## Etapa atual
Etapa 5 — Primeiro build: `con` compilando em background (90.898 .cpp; log extracted/build_con.log).
Depois: corrigir erros, compilar `pcsx2gs.dll` do thirdparty, iniciar ciclo M1.
Etapa 4 CONCLUÍDA (14c5869 + 90.898 .cpp gerados). Etapas 1–3 concluídas (665a9b1, 73998b2, e1e84a5, 5ca5c99).

## Armadilha importante (windows)
`CON` (e PRN/AUX/NUL/COM1-9/LPT1-9) é nome reservado de dispositivo: nenhuma pasta/arquivo pode se
chamar `con` em qualquer componente do caminho — git/MSYS falham com "No such file or directory" e o
PowerShell some com o diretório. A pasta de fontes do port chama `src/port`; o alvo continua `con.exe`.

## Concluído
- Etapa 0 (Auditoria, parcial — docs/AUDIT.md ainda a escrever):
  - runtime/README.md do referência lido: runtime fork do PS2Recomp (TaylorNAlbarnaz, branch sotc-port,
    commit `ac9efa0` 2026-10-01) com IOP/SIF, SPU2, GS do PCSX2, IPU (AC5, Apache-2.0), VU1 AOT (vu1_recomp).
    Hipótese "usar o runtime/ do referência" CONFIRMADA.
- Etapa 1 (CONCLUÍDA, commit 665a9b1):
  - Projeto criado em C:\recomp\CoNRecomp.
  - .gitignore criado ANTES de tudo (regra nº 1): game/, extracted/, build/, bench/, generated? (ver abaixo),
    textures/, mods/, shadercache*/, *.iso/*.bin/*.exe/*.dll.
  - Copiado do referência (só genérico): runtime/ (299 arquivos), thirdparty/ (369 — directx-headers, pcsx2,
    pcsx2-gs, README), tools/ (18 arquivos limpos sem menção ao Urban Reign), docs/licenses/ (AC5 Apache-2.0),
    .gitattributes, LICENSE (GPL-3.0).
  - CoN (próprio): recomp/cn_functions.csv (3.852.170 bytes, mapa do ExportPS2Functions.java, 14.659 funções),
    recomp/cn_config_ghidra_draft.toml (29.489 bytes, formato ran-j — será convertido para formato do fork na
    Etapa 4 como recomp/cn.toml).
  - Adaptados (não copiados verbatim): CMakeLists.txt (raiz, CON_*), build.py (con.exe, CON_JOBS/CON_DEPS),
    src/CMakeLists.txt (placeholder; completo no M1).
- Etapa 2 (em andamento): SYSTEM.CNF lido (SLUS_205.65 CONFIRMADO, VER 1.02, NTSC); ELF extraído
  (9.249.659 bytes, SHA-256 9A60F13C…56FF0), IDÊNTICO ao analisado no Ghidra; tools/win/iso_read.ps1 escrito;
  docs/GAME.md esboçado; listagem completa do disco correndo em background.

## Excluído do referência (urban-específico; reescrever/adaptar depois)
- tools/: find_sdk_functions.py, generate.py, ghidra_functions.py, release.py, frame_times.py, task_list.py,
  vif_chain.py, autotest/bench_state.py, autotest/drive.py, ghidra/DecompileAt.java, status/generate.py,
  win/capture_*.ps1 (3) — contêm nomes/endereços do Urban Reign.
- src/urban/* (29 arquivos): hle_bindings, function_hooks, anim_decoder, aspect, mods, mod_loader, game_disc etc.
  Os genéricos de infraestrutura (log, sha256, hang_dump, thread_policy, call_profile/trace, native_profile) serão
  trazidos/adaptados no M1 conforme necessário.
- .recomp.json, mods/, modsdk/, launcher/, src/res/urban.rc, docs/ específicos (GAME.md, PROGRESS.md, ROADMAP.md,
  status/).

## Falhou / pendente de ambiente (verificar na etapa do build)
- Python não instalado no Windows (só stub WindowsApps) — necessário 3.10+ para build.py/tools.
- WSL "decomp-wtf": apt interrompido (dpkg --configure -a pode ser preciso). Não crítico.
- JDK 27 instalado (guia pede JDK 21); Ghidra 12.1.4 + ger-2.1.38 já funcionou com JDK 27 — sem problema até agora.

## Próximos passos
1. Etapa 2 (faltam): listagem/IRX/vídeos do disc_listing.txt + complemento de docs/GAME.md + commit
   (iso_read.ps1, GAME.md, atualizações).
2. AVISAR USUÁRIO: Python 3.10+ não está instalado — necessário para Etapa 4 (build.py generate). Instrução:
   instalar de https://www.python.org/downloads/ (3.12, marcar "Add python.exe to PATH") OU
   `winget install -e --id Python.Python.3.12` (o usuário decide; regra: sem instalar sem avisar).
3. Etapa 3: refinar mapa; SDK por assinatura; VU1/RNG/rede/multitap/save/GameIndex.
4. Etapa 4: recomp/cn.toml formato do fork; tools/generate.py adaptado; primeira geração (precisa Python + ps2_recomp.exe).

## Comandos úteis
- cd C:\recomp\CoNRecomp
- python build.py                  (menu)
- python build.py generate
- python build.py build
- ISO referência: C:\PROJETOS\Champions Recomp for PC - PS2RECOMP\Ferramentas Úteis\pcsx2-v2.9.131-windows-x64-Qt\ISO E BIOS\Champions of Norrath (USA).iso
- ELF analisado no Ghidra: C:\Users\Alessandro\Decomp\slus-20565\game.elf (entry 0x00100008 — verificar igualdade na Etapa 2)
- Referência-fonte: C:\PROJETOS\Champions Recomp for PC - PS2RECOMP\Ferramentas Úteis\urban-reign-recomp-main
