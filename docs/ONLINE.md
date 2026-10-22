# ONLINE — modo LAN (jogar com o irmão na mesma casa)

**Meta do usuário**: um **modo LAN funcional** na mesma internet (mesma casa). Nada de internet pública,
port forwarding ou servidores oficiais (a SOE não existe mais).

## O que a comunidade já provou (fontes verificadas em 2026-10-21)
1. **DNAS bypass**: patch `.pnach` (PCSX2) "CON DNAS Bypass - NTSC" para o SLUS-20565:
   `D1228C3C/00000006`, `01228C30/00000000`, `D1228C3C/00000006`, `01228C3C/00000005` (formato
   `patch=1,EE,<addr>,extended,<valor>`). → No nosso port isso vira um **patch nativo na inicialização**
   (mesmos endereços/valores — são do nosso ELF, não de outro jogo).
2. **Servidor privado**: *Champions Reborn Server* (HikikoMarmy, release v1.0.0, Windows x64) — o host
   roda o server e os clientes apontam para o IP dele.
3. **PCSX2 prova o caminho**: Ethernet Device Type = Sockets; DNS interno `con.soegames.net` → IP do
   host; jogo em `Start New Game > Internet Game`; **UDP 3000** (só necessário para internet, não para
   LAN). Rede do jogo: `SCE/Ethernet (Network Adaptor)`, IP/DNS automáticos.
4. **Open-Champions** (projeto irmão, MIT): reconstruiu o protocolo de rede do motor em C++
   (codec/sessão/tick, `champions-netwire`) — referência válida do protocolo; e o parser
   `champions-data` (GOB/LMP). Repositório clonado em `the Open-Champions/open-champions-godot repository (MIT)`.
5. **PSRewired** (psrewired.com): serviço que rehospeda tráfego SOE (alternativa pública ao server
   próprio).

## Diagnóstico do nosso runtime (evidência)
- `ps2xIOP` conhece os módulos do IOP: a lista builtin já inclui **`dev9`, `netman`, `ps2ip`** — mas
  **não existe ponte de rede** (grep por socket/UDP/WSA no runtime: zero; só debug UDP na Vita).
- O disco traz os IRX necessários: `DEV9.IRX`, `SMAP.IRX`, `INET.IRX`, `PPP/PPPOE` (MODULES/ e BG/0MOD/).
- O jogo resolve nomes (o PCSX2 usa DNS interno) → nosso port precisa de um **resolvedor próprio**
  (mapear `con.soegames.net` → IP do host no `config.ini`).

## Plano (feature "LAN" — depois do M2)

| # | tarefa | tamanho | dependência |
|---|---|---|---|
| 1 | **Bridge de sockets no runtime**: DEV9/SMAP → UDP do host (send/recv, ~300 linhas) | médio | — |
| 2 | Rodar os `SMAP.IRX`/`INET.IRX` do disco no IOP emulado (ou HLE do SMAP, como o PCSX2) | médio | M2 |
| 3 | **DNS override** no port (`config.ini`: `Net.DnsHost=con.soegames.net`, `Net.DnsIp=<IP do host>`) | pequeno | 2 |
| 4 | **DNAS bypass nativo** (valores do patch da comunidade, aplicados no boot) | pequeno | M1 |
| 5 | Script/guia: *Champions Reborn Server* no host + clientes com o IP dele | pequeno | 1–4 |
| 6 | Teste LAN real (2 PCs, mesma casa): criar personagem → Internet Game → host/join | — | 1–5 |

## Riscos e honestidade
- O item 1–2 é o único trabalho real (o resto é configuração): alguns dias de trabalho **depois** do M2,
  quando o jogo bootar e a tela de rede aparecer.
- Caminho alternativo se o SMAP do disco não cooperar: **HLE do SMAP** (implementar o driver mínimo no
  runtime, como o PCSX2 faz em DEV9/Sessions) — mais controle, mesmo objetivo.
- Multiplayer **local** (2–4 jogadores, multitap) é o M6 e **não depende de nada disso** — se quiser
  jogar junto antes, esse é o caminho mais curto.

## Como vai funcionar para o usuário (desenho final)
1. PC do irmão A: roda `con.exe` + abre o *Champions Reborn Server* (ou usa o PSRewired).
2. PC B: roda `con.exe`.
3. Nos dois: `config.ini` com o IP do host A (DNS override) — o jogo veja `con.soegames.net` = IP do A.
4. `Start New Game > Internet Game > Host/Join`. Sem porta encaminhada (LAN).
