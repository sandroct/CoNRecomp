# ECOSSISTEMA — recursos da comunidade para o recomp (mapeamento)

Levantamento feito em 2026-10-21 para saber **o que usar de cada projeto** (e o que não serve).
Regra do projeto: nada é copiado sem crédito/licença; só o genérico; valores do nosso jogo vêm da
nossa análise.

## 1. Ports recomp existentes (método comprovado)

| projeto | o que é | valor para nós |status |
|---|---|---|---|
| **urban-reign-recomp** (GPL-3.0) | port Urban Reign PC 1.3.0 (PS2Recomp + runtime + GS PCSX2) | **base do nosso método** (já copiado/adaptado: runtime, CMake, build.py, docs) | em uso |
| **recomp-code/dbz-bt4-recomp** (GPL-3.0) | DBZ BT4 — recomp + online via relay; `ps2EntryRunner.exe` (mesma família do runtime) | **prova de online num runtime recomp**: `PS2X_NET_RELAY/TOKEN/PLAYER/JUMP`, determinismo (`PS2X_FRAMESTEP/FIBERS/RANDSEED/RTC`) | clonado p/ estudo (C:\Users\Alessandro\dbz-bt4-recomp) |
| **recomp-code/bloody-roar4-recomp**, **street-recomp-v3** (GPL-3.0) | ports do mesmo autor | referência extra (estrutura de build/patches) | a clonar se necessário |
| **recomp-code/recomp-online** (GPL-3.0, Go) | lobby + salas + **relay UDP** para ports recomp | padrão de integração launcher→jogo; relay reutilizável | referência |

## 2. Projetos nativos/Decomp do Champions (pesquisa, não nosso método)

| projeto | o que é | valor para nós |
|---|---|---|
| **Open-Champions/open-champions-godot** (MIT) | port nativo Godot+C++ dos dois Champions + parser `champions-data` (GOB/LMP) + **reconstrução do protocolo de rede** | parser para a **tradução** (extrair texto); referência do protocolo p/ online |
| **Open-Champions/champions-of-norrath** (re) | reconstrução do fonte do CoN | referência de funções (cautela: jogo diferente do RTA) |
| **Champions-Reborn-Server** (HikikoMarmy) | **servidor privado** Windows | online/LAN (docs/ONLINE.md) |
| **PSRewired** (psrewired.com) | rehospedagem de tráfego SOE | alternativa pública de servidor |

## 3. Ferramentas e conhecimento

| recurso | valor para nós |
|---|---|
| **Snowblind Archive Extractor** (RealNeco000) | forense de .TEX/.LMP (slices 128 KB, offset 0x810, VAGp) — ajuda a entender os arquivos; **não substitui** o parser deles p/ texto |
| **DNAS bypass .pnach** (comunidade) | patch nativo no port (docs/ONLINE.md) |
| **universal-ps2-recompiler** (SandstoneDev, PS2RC License 1.2) | kit de port para **PSP** com outro recompilador: **não usamos o engine**; valor = metodologia (skills de port/HLE, doutor de deps) — licença não-GPL exige cuidado |
| **ps2-recomp-Agent-SKILL** (hkmodd, 40★) | skill de agente + base de conhecimento PS2Recomp (pipeline, syscalls/stubs, R5900, playbook, PCSX2-MCP) | **metodologia validada** (nosso STATE.md == o PS2_PROJECT_STATE.md deles; triage stubs ret0/ret1; overrides por CRC32) e referência de syscalls/SDK p/ o M1; clone em C:\Users\Alessandro\ps2-recomp-Agent-SKILL |
| **ps2-recompilation-guide** (nate-santos) | guia de estratégia (out/2026): **parcialmente desatualizado** — afirma "nada jogável saiu do recomp", mas ignora a leva de ports recomp de out/2026 (Urban Reign 1.3.0, DBZ BT4, Bloody Roar 4, NBA Street) que já rodam jogos de varejo; utilidade = *checklist* de ferramentas (ps2dis#, PINE/PCSX2-MCP, parallel-gs, renderdoc, splat/objdiff) e de armadilhas (delay slots, float não-IEEE, ELF stripped, override por CRC32) |

## 4. O que já internalizamos do guia (checklist vs. nosso estado)
- Ghidra + ghidra-emotionengine-reloaded ✅ (14.659 funções mapeadas)
- Mapa de funções → recompilador ✅ (`recomp/cn_functions.csv`, merge)
- PCSX2 como ground truth ✅ (ISO verificada; Etapa 3 usa o GameIndex; comparações no M3+)
- Overrides por build/CRC ✅ (SHA-256 do ELF fixado no generate.py)
- Armadilhas conhecidas e tratadas: delay slots (recompiler do fork), float EE (runtime), stub por endereço.

## 5. Próximas ações de pesquisa (quando o build terminar)
1. Estudar `PS2X_NET_*` no runtime do DBZ (nosso runtime **não tem**) → portar a ponte (GPL→GPL, com
   crédito) para o item 1 do docs/ONLINE.md.
2. Testar o `champions-data` do Open-Champions nos nossos `.LMP`/`.GOB` (extração p/ tradução).
3. Verificar se o netcode do CoN fala com o relay (ou precisa do Champions Reborn Server direto).

## 6. Já colhido do ps2-recomp-Agent-SKILL (aplicado ao M1)
- `game_overrides.h` do nosso runtime: `PS2_REGISTER_GAME_OVERRIDE(nome, ELF, entry, CRC32, apply)`
  + `bindAddressHandler(runtime, addr, nome)` — override por jogo **sem tocar no runtime**.
- **Cobertura dos nossos stubs: 431/431** (`tools/hle_coverage.py`): todos têm handler por nome em
  `ps2_call_list.h` (658 handlers). HLE do M1 = tabela de endereços, já gerada
  (`tools/gen_hle_bindings.py` → `src/port/hle_bindings.cpp`, CRC32 `0x7AA91374`).
- Estratégia de triagem (ret0/ret1/reta0) documentada para os 430 `untracked_stubs` caso o boot trave.
- Próximo: PCSX2-MCP (build custom do PCSX2 com DebugServer via MCP) para comparação A/B no M1–M3
  (nossa missão já exige comparar com PCSX2) — estudar `resources/12-pcsx2-mcp-playbook.md` antes.
