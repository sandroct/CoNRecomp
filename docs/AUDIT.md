# AUDIT — Etapa 0 (auditoria de ambiente e materiais)

Data: 2026-10-21 (consolidada ao fim da Etapa 1; itens pendentes marcados).

## 1. Materiais no disco (confirmados)

| item | caminho | estado |
|---|---|---|
| ISO Champions of Norrath (USA) | `C:\PROJETOS\Champions Recomp for PC - PS2RECOMP\Ferramentas Úteis\pcsx2-v2.9.131-windows-x64-Qt\ISO E BIOS\Champions of Norrath (USA).iso` | 7,75 GB; SYSTEM.CNF lido; ELF extraído (SHA-256 bate com o do Ghidra) |
| Referência-fonte (urban-reign-recomp) | `C:\PROJETOS\Champions Recomp for PC - PS2RECOMP\Ferramentas Úteis\urban-reign-recomp-main` | lido; inventário em COPIED_FROM_REFERENCE.md |
| ran-j/PS2Recomp | `C:\Users\Alessandro\PS2Recomp` | usado para (a) CSV do Ghidra (`configs\norrath_functions`, 14.659 funções) e (b) verificação do formato do config `norrath_config.toml` |
| ELF analisado no Ghidra | `C:\Users\Alessandro\Decomp\slus-20565\game.elf` | 9.249.659 B; SHA-256 `9A60F13C…56FF0` (idêntico ao extraído) |
| Release UrbanReignPC-1.3.0 | `...\Ferramentas Úteis\UrbanReignPC-1.3.0` | só referência; NADA copiado (urban.exe/DLLs/mod/texturas) |
| cópia em cn-recomp (antiga) | `C:\Users\Alessandro\cn-recomp` | obsoleta; projeto canônico é `C:\recomp\CoNRecomp` |

## 2. norrath_config.toml / norrath_functions.csv (formato e contagens)

- `norrath_functions` (CSV): cabeçalho `Name,Start,End,Size`; 14.659 funções + ENTRYPOINT
  (0x00100008). Formato de nomes variado: símbolos (`sceGsResetGraph`), `entry_XXXXXXXX`, `sub_XXXXXXXX`.
- `norrath_config.toml` (formato **ran-j/PS2Recomp**, não o do fork): `[general]` com input/output/
  ghidra_output/single_file_output/patch_syscalls=false/patch_cop0=true/patch_cache=true,
  `stubs` (431 entradas `name@addr` — funções do SDK identificadas), `untracked_stubs` (430),
  `skip = []`, e `[ghidra_export]` com os totais.
- Diferenças para o `recomp/urban.toml` do fork: o fork usa `ghidra_map_merge`, `auto_stub_by_name=false`,
  `stubs=[]`, `patch_cop0=false`, `output_worker_threads`, `direct_tail_calls=false`. Decisão:
  `recomp/cn.toml` segue o **formato do fork** (é o recompilador que usamos); o draft ran-j fica em
  `recomp/cn_config_ghidra_draft.toml` como fonte dos endereços dos stubs (Etapa 3/HLE).
- Os 431 stubs do draft (`sceGsResetGraph@0x002D16E8` etc.) batem 1:1 com o CSV → mapa e stubs
  coerentes; hash do ELF idêntico ⇒ **não é necessário refazer o mapa**.

## 3. Ambiente (verificado)

| item | situação |
|---|---|
| VS2022 C++ (cl/MSVC) | BuildTools 19.44.35229 em `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` — OK (sem Community/Professional) |
| CMake + Ninja | 3.31 + Ninja inclusos no BuildTools; usados via `tools/win/vsenv.cmd` (ajustado p/ BuildTools) |
| Python | **instalado 3.12.10** (winget, autorizado). Caminho real: `C:\Users\Alessandro\AppData\Local\Programs\Python\Python312\python.exe` (o stub do WindowsApps ainda vem antes no PATH do shell do agente) |
| GPU DX11 | GPU do usuário assume DX11 (a GS do PCSX2 usa D3D11); não verificado por API — hipótese OK para o M1 |
| Ghidra 12.1.4 + ghidra-emotionengine-reloaded v2.1.38 | já usado para gerar o CSV (JDK 27 funcionou; guia pedia 21 — sem bloqueio até agora) |
| RAM | ~16 GB — calibrou o build: sem PCH, sem `/d2OptimizeHugeFunctions` no lib normal, huge com JOB_POOL 1, `-j 4` (docs/PROGRESS.md E5.7) |
| Disco | C: com >300 GB livres |

## 4. Runtime (decisão)

- Hipótese confirmada lendo `runtime/README.md` do referência: o fork do PS2Recomp
  (TaylorNAlbarnaz/PS2Recomp, branch `sotc-port`, commit `ac9efa0`, 2026-10-01) já contém:
  `ps2xRuntime` (EE/VU/GS), `ps2xIOP` (IOP emulado + RPC/SIF), GS do PCSX2 (`thirdparty/pcsx2-gs`
  → `pcsx2gs.dll`), IPU (do AC5 recomp, Apache-2.0), ferramentas offline (`vu1_recomp` AOT de VU1).
- **Decisão: usar esse runtime/**, copiado para o projeto (GPL-3.0). O recompilador também vem dele
  (compilado com `-DURBAN_BUILD_RECOMP=ON` → `build/recomp/ps2xRecomp/ps2_recomp.exe`).

## 5. Riscos e particularidades registradas

- `CON` é nome reservado de dispositivo no Windows → nenhuma pasta `con` no caminho (sintoma:
  git "No such file or directory"; PowerShell some com o diretório). Pasta do port: `src/port`.
- Disco tem **dois ELFs na raiz**: `SLUS_205.65` (boot) e `NTGUI2.ELF` (5,6 MB — hipótese:
  front-end de rede; SYSTEM.CNF boot só cita SLUS_205.65).
- Regiões do DNAS (blob cifrado) viram "código" no mapa → 173 arquivos `dnas2`, 2 monstruosos
  (stubados + `skip` no cn.toml; documentado em docs/GAME.md).
- WSL "decomp-wql": apt interrompido por restarts; **não usado** no método atual (tudo nativo Windows).
