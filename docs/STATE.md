# CoNRecomp — STATE

Data: 2026-10-21

## Etapa atual
Etapa 1 (Consolidação) — em andamento; git init/commit pendente.

## Concluído
- Etapa 0 (Auditoria, parcial — ver docs/AUDIT.md quando escrito):
  - runtime/README.md do referência lido: runtime fork do PS2Recomp (TaylorNAlbarnaz, branch sotc-port,
    commit ac013f/`ac9efa0` 2026-10-01) com IOP/SIF, SPU2, GS do PCSX2, IPU (AC5, Apache-2.0), VU1 AOT (vu1_recomp).
    Hipótese "usar o runtime/ do referência" CONFIRMADA.
- Etapa 1:
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
1. TERMINAR Etapa 1: README.md, THIRD-PARTY.md, docs/COPIED_FROM_REFERENCE.md, docs/AUDIT.md (Etapa 0), git init,
   1º commit (.gitignore já no lugar).
2. Etapa 2: SYSTEM.CNF → nome do ELF (hipótese SLUS_205.65), extração em extracted/, SHA-256, docs/GAME.md,
   lista de IRX/vídeos.
3. Etapa 3: refinar mapa (tools/ghidra_functions.py adaptado), SDK por assinatura, VU1, RNG, save, GameIndex.
4. Etapa 4: recomp/cn.toml (formato do fork), tools/generate.py adaptado, primeira geração.

## Comandos úteis
- cd C:\recomp\CoNRecomp
- python build.py                  (menu)
- python build.py generate
- python build.py build
- ISO referência: C:\PROJETOS\Champions Recomp for PC - PS2RECOMP\Ferramentas Úteis\pcsx2-v2.9.131-windows-x64-Qt\ISO E BIOS\Champions of Norrath (USA).iso
- ELF analisado no Ghidra: C:\Users\Alessandro\Decomp\slus-20565\game.elf (entry 0x00100008 — verificar igualdade na Etapa 2)
- Referência-fonte: C:\PROJETOS\Champions Recomp for PC - PS2RECOMP\Ferramentas Úteis\urban-reign-recomp-main
