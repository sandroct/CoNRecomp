# CoNRecomp

**Champions of Norrath: Realms of EverQuest** (PlayStation 2, EUA — **SLUS-20565**) como um programa
nativo de Windows, via recompilação estática antecipada.

> 🇺🇸 **English:** see [README.md](README.md).

## O que é

- O código MIPS R5900EE do jogo é traduzido antecipadamente para C++ (`generated/` — 90.898 arquivos,
  **nunca** neste repositório) e compilado num único executável Windows (`con.exe`).
- Os módulos de IOP que vêm no disco (`IRX`) rodam sob o IOP emulado (`runtime/ps2xIOP`).
- Os gráficos passam pela **GS do PCSX2** (compilada de `thirdparty/pcsx2-gs` como `pcsx2gs.dll`).
- Microprogramas de VU1 podem ser compilados antecipadamente (`vu1_recomp`, `runtime/ps2xTest`).
- Método e estrutura seguem o [urban-reign-recomp](https://github.com/) (GPL-3.0); **todo valor
  específico do jogo** (mapa de funções, config de recompilação, bindings de HLE) vem da análise do
  próprio *Champions of Norrath* — nada específico de outro port é copiado.

## Situação

Em desenvolvimento — engenharia em público, não é uma release.

| etapa | estado |
|---|---|
| 0–4 — auditoria, estrutura, identificação do disco/ELF (SHA-256 conferido), análise, **recompilação do código do jogo (90.898 funções)** | ✅ feito |
| 5 — primeira compilação do `con.exe` (compilando agora, ~15% de 90.902 objetos) | 🔄 em andamento |
| 6+ — marcos de runtime M1 (boot) … M8 (autotest vs. PCSX2) | ⬜ planejado |

Tudo documentado: [`docs/STATE.md`](docs/STATE.md) (etapa atual),
[`docs/PROGRESS.md`](docs/PROGRESS.md) (log completo), [`docs/GAME.md`](docs/GAME.md) (fatos do jogo),
[`docs/AUDIT.md`](docs/AUDIT.md) (auditoria do ambiente).

## Compilando

1. Coloque a **sua própria** imagem de disco de Champions of Norrath (EUA) em `game/`.
2. `python build.py` — menu numerado: `generate` recompila o código do jogo, `build` compila o `con.exe`
   (a primeira compilação leva horas numa máquina de 16 GB), `play` roda.

Requisitos: Windows 10/11 64 bits, Visual Studio 2022 com C++ (CMake/Ninja inclusos), Python 3.10+,
GPU com DX11, ~30 GB de disco.

## Destaques do roteiro

- **M1** boot até o menu principal (HLE por endereço já pronto: 431/431 stubs do SDK ligados a handlers
  do runtime).
- **Multiplayer local** (multitap, 2–4 jogadores) no M6.
- **LAN/online**: plano em [`docs/ONLINE.md`](docs/ONLINE.md) (jogar em casa com servidor privado — a
  comunidade já provou o caminho com PCSX2 + *Champions Reborn Server*).
- **Tradução para português do Brasil**: plano em [`docs/L10N.md`](docs/L10N.md) (o motor já suporta
  `pt` na tabela de idiomas; o disco traz pacotes de texto EN/FR/DE/SP).

## Créditos

- Família do runtime/recompilador: [ran-j/PS2Recomp](https://github.com/ran-j/PS2Recomp) e o fork
  `sotc-port` (TaylorNAlbarnaz) — GPL-3.0.
- GS: [PCSX2](https://pcsx2.net/) — GPL-3.0. Decodificador IPU: recompilação estática de Ace Combat 5 —
  Apache-2.0.
- Análise: Ghidra + [ghidra-emotionengine-reloaded](https://github.com/chaoticgd/ghidra-emotionengine-reloaded).
- Referência de método: urban-reign-recomp (GPL-3.0). Recursos da comunidade mapeados em
  [`docs/ECOSYSTEM.md`](docs/ECOSYSTEM.md) (ports do recomp-code, Open-Champions, Champions Reborn
  Server, patches de DNAS, ferramentas de arquivos Snowblind).

## Licença

GPL-3.0 ([`LICENSE`](LICENSE)). Componentes de terceiros e suas licenças: [`THIRD-PARTY.md`](THIRD-PARTY.md).

**Nenhum dado de jogo está neste repositório.** A imagem de disco, BIOS, módulos IRX, assets e o código
gerado pertencem ao usuário e nunca são commitados ou distribuídos. Use o seu próprio disco, obtido
legalmente.
