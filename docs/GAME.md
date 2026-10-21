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
  - **Igual por SHA-256 ao ELF analisado no Ghidra** (`C:\Users\Alessandro\Decomp\slus-20565\game.elf`) → o
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

## A fazer (Etapa 3)
- Funções do SDK por assinatura (libdma, libgraph, libpad, libmtap, libsdr, libcdvd, libmc, libipu, libmpeg...)
  — cross-reference com Urban Reign (SDK 2.8) e SotC (SDK 3.0, com símbolos).
- Tabelas de salto e funções só por ponteiro (os 430 untracked_stubs).
- VU1: onde microprogramas são enviados (form fields GFX?). — pendente
- RNG/semente da geração de fases. — pendente
- Código de rede, multitap, formato do save. — pendente
- Entrada do serial no GameIndex do PCSX2 (correções de GS, SLUS-20565). — pendente
