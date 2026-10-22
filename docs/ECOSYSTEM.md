# ECOSSISTEMA — recursos da comunidade para o recomp (mapeamento)

Levantamento feito em 2026-10-21 para saber **o que usar de cada projeto** (e o que não serve).
Regra do projeto: nada é copiado sem crédito/licença; só o genérico; valores do nosso jogo vêm da
nossa análise.

## 1. Ports recomp existentes (método comprovado)

| projeto | o que é | valor para nós |status |
|---|---|---|---|
| **urban-reign-recomp** (GPL-3.0) | port Urban Reign PC 1.3.0 (PS2Recomp + runtime + GS PCSX2) | **base do nosso método** (já copiado/adaptado: runtime, CMake, build.py, docs) | em uso |
| **recomp-code/dbz-bt4-recomp** (GPL-3.0) | DBZ BT4 — recomp + online via relay; `ps2EntryRunner.exe` (mesma família do runtime) | **prova de online num runtime recomp**: `PS2X_NET_RELAY/TOKEN/PLAYER/JUMP`, determinismo (`PS2X_FRAMESTEP/FIBERS/RANDSEED/RTC`) | clonado p/ estudo (the recomp-code/dbz-bt4-recomp repository (GPL-3.0)) |
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
| **ps2-recomp-Agent-SKILL** (hkmodd, 40★) | skill de agente + base de conhecimento PS2Recomp (pipeline, syscalls/stubs, R5900, playbook, PCSX2-MCP) | **metodologia validada** (nosso STATE.md == o PS2_PROJECT_STATE.md deles; triage stubs ret0/ret1; overrides por CRC32) e referência de syscalls/SDK p/ o M1; clone em the hkmodd/ps2-recomp-Agent-SKILL repository |
| **ps2-recompilation-guide** (nate-santos) | guia de estratégia (out/2026): **parcialmente desatualizado** — afirma "nada jogável saiu do recomp", mas ignora a leva de ports recomp de out/2026 (Urban Reign 1.3.0, DBZ BT4, Bloody Roar 4, NBA Street) que já rodam jogos de varejo; utilidade = *checklist* de ferramentas (ps2dis#, PINE/PCSX2-MCP, parallel-gs, renderdoc, splat/objdiff) e de armadilhas (delay slots, float não-IEEE, ELF stripped, override por CRC32) |

## 4. Referências de hardware e documentação
- **ps2tek** (psi-rockin.github.io/ps2tek) — a referência definitiva do hardware do PS2 (mapas de
  memória, EE/COP0/COP1, GIF/GS, DMAC, VIF, VU, IPU, SIF, IOP, BIOS). É a fonte que os devs do PCSX2
  usam. **Indexada no XERJ** (prefixo `ps2tek`, cópia local em `C:\Users\Alessandro\refs\ps2tek\`).
  Uso no projeto: quando o log do runtime acusar um registro/instrução (M1–M4), consultar o índice
  antes de abrir a página.
- **n64recomp/xenonrecomp** — a linhagem do método (citada pelo guia de estratégia).

## 5. XERJ — indexador de código para agentes (instalado 2026-10-21)
- O que é: motor de busca "AI-native" (Rust, Apache-2.0, binário único) que indexa código (tree-sitter)
  e documentos; consulta por HTTP (:9200, compatível Elasticsearch) — o agente **pergunta ao índice**
  em vez de ler arquivos inteiros (economia tokens de contexto).
- Instalado em: `C:\Users\Alessandro\bin\xerj\xerj-1.0.0-rc.97-x86_64-pc-windows-msvc\xerj.exe`
  (hash do release verificado; nó local roda destacado com `--insecure`).
- Corpora indexados: **dbz-bt4-recomp** (76 arquivos de código — onde está a implementação do
  `PS2X_NET_*`), **nosso `runtime/`** (prefixo `conrt`), **ps2-recomp-Agent-SKILL** (prefixo `skill`),
  **ps2tek** (prefixo `ps2tek` — hardware do PS2).
- Uso: `xerj search "<frase>"` (busca lexical com `defs` AST) e `xerj def <símbolo>` (vai para a
  definição). Exemplo que já rendeu: *"PS2X_NET_RELAY netplay relay"* achou
  `patches/bt3recomp-bt4mod-windows.patch` com a implementação completa do relay.

## 5. O que já internalizamos do guia (checklist vs. nosso estado)
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
