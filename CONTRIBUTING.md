# Contributing / Como contribuir

## 🇺🇸 English

Thanks for your interest in CoNRecomp! This is a static-recompilation port of Champions of Norrath
(SLUS-20565) to Windows, built with the PS2Recomp method.

**Ground rules**

1. **Bring your own disc.** Never commit, upload, or link to the ISO, BIOS, ELF, IRX modules, assets,
   shader caches, or the generated (`generated/`) code. Pull requests with game data are closed.
2. **No game content in code review either.** Logs and traces are fine — strip them to the relevant
   lines.
3. **Evidence over claims.** Statements about the game must cite a binary, log, or trace. Mark
   hypotheses as "hypothesis".
4. **Keep the structure.** The method mirrors urban-reign-recomp: small generic pieces, the recompiler
   config in `recomp/`, per-game data in the port sources, tests/comparisons against PCSX2.

**Useful contributions**

- HLE handler fixes (runtime stubs bound by address — see `src/port/hle_bindings.cpp`)
- GS/GameIndex corrections (PCSX2 serial `BASLUS-20565`)
- Build fixes on low-RAM machines (MSVC heap handling)
- Documentation, translations of the docs themselves
- Tools: extraction, comparison, testing (see `tools/`)

**Workflow**: fork → branch → commit with a clear message (the log style of `docs/PROGRESS.md`:
date, command, result, address, reason) → pull request describing the evidence.

## 🇧🇷 Português (Brasil)

Obrigado pelo interesse no CoNRecomp! É um port por recompilação estática de Champions of Norrath
(SLUS-20565) para Windows, com o método PS2Recomp.

**Regras básicas**

1. **Use o seu próprio disco.** Nunca comite, suba ou linke ISO, BIOS, ELF, módulos IRX, assets, cache
   de shaders ou o código gerado (`generated/`). PRs com dados do jogo são fechados.
2. **Nada de conteúdo do jogo em revisões de código também.** Logs e traces valem — mas só o trecho
   relevante.
3. **Evidência acima de afirmação.** Toda afirmação sobre o jogo precisa citar binário, log ou trace.
   Hipótese se marca como "hipótese".
4. **Mantenha a estrutura.** O método espelha o urban-reign-recomp: peças genéricas pequenas, config do
   recompilador em `recomp/`, dados do jogo nas fontes do port, testes/comparações contra o PCSX2.

**Contribuições úteis**

- Correções de handlers HLE (stubs do runtime ligados por endereço — veja `src/port/hle_bindings.cpp`)
- Correções de GS/GameIndex (serial do PCSX2 `BASLUS-20565`)
- Ajustes de build em máquinas com pouca RAM (tratamento do heap do MSVC)
- Documentação e traduções dos próprios docs
- Ferramentas: extração, comparação, teste (veja `tools/`)

**Fluxo**: fork → branch → commit com mensagem clara (estilo do log de `docs/PROGRESS.md`:
data, comando, resultado, endereço, motivo) → pull request descrevendo a evidência.
