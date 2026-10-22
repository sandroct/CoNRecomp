# GAME — Champions of Norrath: Realms of EverQuest (USA, SLUS-20565)

Fatos confirmados (evidência) vs. hipóteses marcadas.

## Identificação e Paleo disco (EVIDÊNCIA — leitura direta da ISO)
- Disco: `Champions of Norrath (USA).iso`, 7.75 GB (hipótese: DVD-DL/boot4; tamanho não-obrigatório a provar
  que é dual-layer — confirmar estrutura UDF/ISO depois se necessário).
- `SYSTEM.CNF`:
  - `BOOT2 = cdrom0:\SLUS_205.65;1` → executável principal: **SLUS_205.65**
  - `VER = 1.02`
  - `VMODE = NTSC`
- ELF principal: `extracted/SLUS_205.65`
  - tamanho: 9.249.659 bytes
  - SHA-256: `9A60F13C5C316D619A288821CC9496CC61864106B7872B52EE141CAED3756FF0`
  - entry point: 0x00100008 (do mapa de funções, ENTRYPOINT)
  - **Igual por SHA-256 ao ELF analisado no Ghidra** (`your Ghidra analysis of the SLUS-20565 ELF\game.elf`) → o
    mapa `recomp/cn_functions.csv` (14.659 funções, 431 stubs) gerado por `ExportPS2Functions.java` é válido
    para este disco.

## Mapa de funções
- `recomp/cn_functions.csv` — colunas `Name,Start,End,Size`; exemplo de cabeçalho:
  `ENTRYPOINT,0x00100008,0x00100248,576`.
- `recomp/cn_config_ghidra_draft.toml` — config no formato do **ran-j/PS2Recomp** (draft); será convertida para
  o formato esperado pelo recompilador do fork (`recomp/cn.toml`) na Etapa 4.

## Conteúdo do disco (extracted/disc_listing.txt, 674 entradas, 13/root + DATA/DATA2/MODULES/MODULES2/NETGUI/NGC/BG/CONF)
- Raiz: `SLUS_205.65` (ELF principal, 9.249.659 B), **`NTGUI2.ELF` (5.639.828 B)** (hipótese: front-end
  GUI/rede carregado pelo jogo — a confirmar), `SYSTEM.CNF` (57 B).
- `MODULES/` + `MODULES2/` + `BG/0MOD/`: módulos IOP em `.IRX`/`.IMG` (63 IRX no total), entre eles:
  - kernel/jogo: `SIO2MAN`, `SIO2D`, `PADMAN`, `MTAPMAN` (multitap!), `MCMAN`, `MCSERV`, `MSIFRPC`, `LIBSD`,
    `CDVDSTM`, `ATAD`, `IOPRP280.IMG`, `HELLO.IRX`
  - áudio: `SNDDRV`, `SDRDRV`, `LGAUD`, `MODHSYN`, `MODMIDI`, `EZMIDI`
  - rede/online: `SMAP`, `INET`, `DEV9`, `PPP`, `PPPOE`, `CXTMDM`, `SPDUART`, `INETCTL`, `EZNETCTL`,
    `EENETCTL`, `NETCNF`, `NETCNFIF`, `EZNETCNF`, `LIBNET`, `MCSERV`, `NTGUI2.IRX`, **`DNAS280.IMG`/`DNAS300.IMG`**
    (autenticação online SOE/Sony — hipótese a confirmar no binário)
  - USB: `USBD`, `USBINIT`, `USBMLOAD`, `USBKB`, `LGKBM`
- `DATA/` e `BG/DATA/` (dados de jogo, formato Snowblind):
  - `.LMP` (376 — mapas/modelos "Lump"), `.TEX` (25, até 48 MB — texturas), `.GOB` (25), `.WAD` (2),
    `.VAT`/`.IMG` (áudio), `.CSF`/`.ESF`, `.XML` (22), `.CNF` (20), `.RGB`, `.PF`, `.BIN`, `.DAT`
  - nomes indicam zonas de Norrath: `FAYDARK2`, `D_HATE`, `D_HATE2`, `ORCCAVE1`, `CRUSHER`, `ANTHILL2`,
    `PIPES`, `FIRLOW`/`FILOW` (Crushbone/Firiona?) — hipótese: mapas clássicos do EQ
- `CONF/NET/`: perfis de rede `.CNF` (NET001/003, DEV*, IFC*, DIAL, USB_CXTM) + `NET.DB`, `VSSVER.SCC`.
- Vídeo/IPU: nenhum arquivo de vídeo óbvio (sem .PSS/.IPU na raiz do disco) — a confirmar: os .VAT podem
  conter streams.

## Acesso a disco pelo ELF (M2 — caminhos fixos no binário)
Todas as referências `cdrom0:\` (14 strings) no ELF:
- `cdrom0:\SLUS_205.65;1` (o próprio boot) e `cdrom0:\NTGUI2.ELF;1` (**segundo executável carregado — GUI de rede**)
- `cdrom0:\MODULES\%s;1`, `cdrom0:\MODULES2\%s;1`, `cdrom0:\BG\0MOD\%s;1`, `cdrom0:\bg\0mod\%s;1` (IRX por nome montado)
- `cdrom0:\MODULES\DNAS255.IMG;1` e `cdrom0:\BG\0MOD\DNAS280.IMG;1` (autenticação; disco traz DNAS280/300 — **strings
  podem ser de build antiga; conferir no M2**)
- `cdrom0:\CONF\NET\DIAL_CXT.CNF`, `DIAL_SPD.CNF`, `CONF\NETICON\JAPANESE\*`, `MODULES\ICON.SYS`, `MODULES\SYS_NET.ICO`
- `cdrom0:\%s;1` (genérico) → dados de jogo (.LMP/.TEX/.VAT) provavelmente lidos por LBA/fileio bruto ou por esse
  genérico com nome montado — a confirmar no M1/M2 pelos logs de IOP RPC.

## A fazer (Etapa 3)
- Funções do SDK por assinatura (libdma, libgraph, libpad, libmtap, libsdr, libcdvd, libmc, libipu, libmpeg...)
  — cross-reference com Urban Reign (SDK 2.8) e SotC (SDK 3.0, com símbolos).
- Tabelas de salto e funções só por ponteiro (os 430 untracked_stubs).
- VU1: onde microprogramas são enviados (form fields GFX?). — pendente
- RNG/semente da geração de fases. — pendente
- Código de rede, multitap, formato do save. — pendente
- Entrada do serial no GameIndex do PCSX2 (correções de GS, SLUS-20565). — pendente

## Achados por strings (extracted/elf_strings.txt, tools/elf_strings.py)
- Serial/GameIndex: `BASLUS-20565`, `/BASLUS-20565`, `BASLUS-20565EQONLINE`, `BASLUS-20470EQONLINE` →
  **chave do GameIndex do PCSX2: BASLUS-20565** (+ variante EQONLINE); SLUS-20470 = segunda title ID no binário
  (hipótese: boot "online" compartilhado; confirmar durante o M1/M2).
- VU1: `EXPL_vu1skin_vsm_meshClipLoop__MAIN_LOOP`, `EXPL_vu1world_vsm_worldMeshClipLoop`,
  `EXPL_vu1skin_vsm_refsqueezemeshClipLoop` → microprogramas de **skinning/mundo em VSM**; `PsIIlibvu0  2800`
  (libs SN Systems).
- Rede/online (M2 — stub necessário): `VIUDPSocket::SendTo/RecvFrom`, `Connecting to SOEGames.net...`,
  `Sony Online Entertainment Credits:`, `soe_splash.tex`, `soe_splash_url.tex`, USOCKET_TYPE_UDP;
  strings citam `cdrom0:\MODULES\DNAS255.IMG` (disco traz DNAS280/DNAS300 — verificar qual é carregado).
- Save (M7): `MC_SaveGameBrowser`, `menu_savegame`, `icon.sys`, `WORDS.TXT read from memory card`,
  `Writing session key (%ld) to memory card` (chave de sessão DNAS gravada no MC).
- Engine/dados: `equipmnt.lmp`, `championsbreastplate+requiredclass1+dexteritybonus30` → formato "nome+atributos"
  dos itens nos .LMP; `Material Palettes/Allocations: %d` (log do engine de materiais).
- Entrada: entry 0x00100008 (boot exit stub "exit@0x00100248"); IRX citados no ELF: mcman.irx, mcserv.irx,
  netcnf.irx, msifrpc.irx, eznetcnf.irx, MCMAN.IRX, MCSERV.IRX, PPP.IRX, PPPOE.IRX, SMAP.IRX, "unloading smap".
- Áudio: `liblgaud version 1.09.004, built on Sep 15 2003` (build de 2003 do audio).

## Visão do jogador (manual oficial — resumo de fatos, sem citar texto protegido)
Fonte: manual EN do jogo (23 págs., lido por extração de imagens em 2026-10-21).
- **Fluxo**: intro cinematográfica → **Main Menu** (Start New Game / Load Saved Game / Options / Credits) →
  **Normal Game** (1–4 jogadores no mesmo PS2) ou **Internet Game**.
- **Personagens (5 classes)**: Barbarian Warrior, Wood Elf Ranger, High Elf Cleric, Erudite Wizard,
  Dark Elf Shadowknight; + gênero, aparência (pele/cabelo), 20 pontos em Strength/Intelligence/
  Dexterity/Stamina, nome em grid alfabético (X + DONE). Bate com os `.lmp` de raça no disco
  (`eru_male`, `elf_fdkl`...) e as strings do ELF (`barbarian`, `wood_elf`, `high_elf`, `dark_elf`).
- **Dificuldades**: Adventurous / Courageous (nível 20–35) / Champion (35–50).
- **Controles**: dpad navega menu + X seleciona + ● volta; START pausa; SELECT inventário;
  X ataca; ■ interage/pega itens; ●/▲ habilidades (menu radial); R2 poção de vida, L2 poção de mana;
  R3 automap; L1 troca melee/à distância.
- **Combate/progressão**: vida/mana regeneram; morte volta ao último save point (pedestal);
  quest journal; level up com pontos de atributo/habilidades; loot com limite de carga (Strength).
- **Multiplayer (confirma o M6 e o ONLINE)**: co-op local com **Multitap (até 4)**; importar
  personagens de saves do memory card; **Internet Game**: host ou join via **SOEGames.net**, com
  configuração de rede (ISP) salva no memory card (8 MB); headset USB para voz (stack USB no disco).

## GameIndex do PCSX2 (Etapa 3 — correções oficiais, evidência: pcsx2/bin/resources/GameIndex.yaml)

**SLUS-20565 (a nossa, NTSC-U)** — compat 5:
- `gameFixes`: `BlitInternalFPSHack` (corrige detecção de FPS interna em algumas áreas)
- `gsHWFixes`: `textureInsideRT: 1` (corrige "metade direita"), `estimateTextureRegion: 1` e
  `gpuPaletteConversion: 1` (performance + tamanho do hash cache)

**SLES-52325 (PAL-M3, "Realms of EverQuest")** e **SLES-52373 (PAL-E-S)**: os mesmos acima **mais**
`roundSprite: 1` (linhas em menus/HUD), `halfPixelOffset: 2` (lixo nas bordas), e a M3 também
`PCRTCOverscan: 1` (imagem fora da tela). → Se um dia suportarmos as PAL, usar a lista completa.

**Ação no port (M3)**: como usamos a GS do PCSX2 (`pcsx2gs.dll`), esses `gsHWFixes` precisam ser
aplicados via a API da wrapper (config da GS) na inicialização — e `BlitInternalFPSHack` verificar se
deve ser hook no jogo ou settings do host. Especificado em docs/ROADMAP.md (M3).

## Visão do jogador (manual oficial — resumo de fatos, sem citar texto protegido)
431 stubs nomeados por endereço (extracted/sdk_groups.txt, tools/stub_groups.py):
- SIF/RPC (IOP): 52 · kernel EE (threads/dma/intc/sema): ~194 · libpad (+multitap): 29 · libmpeg: 25
- libcdvd: 23 · libmc (memory card): 21 · libdma: 15 · GS (libgraph): 14 + misc 1 · VU/kernel: 41
- libipu: 4 · libsdr: 4 · fileio IOP: 4 · s kernel EE: 2 · outros 3
- + 430 untracked_stubs (ponteiros sem dono — cobertos pelo merge do mapa do Ghidra na geração).
- [ghidra_export] do draft: function_count 14.659; csv_record_count 89.490; runtime_call 656 nomes.
  Esses endereços são os candidatos a `replace` por handlers nativos no M1 (HLE por endereço).
