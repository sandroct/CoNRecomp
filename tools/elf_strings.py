#!/usr/bin/env python3
"""elf_strings.py - scan an stripped PS2 ELF (or any binary) for interesting strings.

Generic analysis tool for the CoNRecomp project: categorizes strings that matter to the port
(engine markers, SDK calls, network/DNAS, serial, save/multitap hints). Pure stdlib.

Usage:
    python tools/elf_strings.py <elf-or-binary> [-o out.txt] [--min 6] [--max-hits 40]
"""
import argparse
import re
import sys
from pathlib import Path

CATEGORIES = {
    "engine": r"(?i)\b(snowblind|dark alliance|champions|everquest|zoneallast|lmp|\.lmp|\.gob|\.wad|\.vat)",
    "sdk": r"(?i)\b(sce[A-Z]\w+|libdma|libgraph|libpkt|libpad|libmtap|libsdr|libcdvd|libscf|libipu|libmpeg|libmc|gss?param|ioprp)",
    "serial": r"(?i)(slus[-_ ]?\d{5}|scps|sces|ntsc|pal|ver(?:sion)?\s*[:=]?\s*\d)",
    "net": r"(?i)(dnas|nettcp|udp|tcpip|smap|atad|pppoe|ppp|sif(rpc|cmd)?|rpc\.?all|netcnf|sony|verant|soe)",
    "audio": r"(?i)(\.vag|\.ss2|\.snd|snddrv|sdrdrv|libsd|spu[0-9]|iop.?mod)",
    "video": r"(?i)(\.ipu|mpeg|mpg|\.pss|ipu\.out)",
    "save": r"(?i)(memory.?card|mc0|mc1|mcserv|mcman|savegame|\.sav\b|icon\.sys|biscard)",
    "pad_mtap": r"(?i)(mtap|multitap|padman|sio2|game.?pad|port.?open)",
    "gs_vu": r"(?i)(vif[01]|vu[01]|gif\.out|gs\.out|xgkick|mip?\.vif)",
}
COMPILED = {k: re.compile(v) for k, v in CATEGORIES.items()}
PRINTABLE = re.compile(rb"[\x20-\x7e]+")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("-o", "--out", default="")
    ap.add_argument("--min", type=int, default=6)
    ap.add_argument("--max-hits", type=int, default=40)
    args = ap.parse_args()

    data = Path(args.path).read_bytes()
    out = open(args.out, "w", encoding="utf-8") if args.out else sys.stdout
    try:
        cat_hits = {k: [] for k in CATEGORIES}
        total = 0
        for m in PRINTABLE.finditer(data):
            s = m.group().decode("ascii").strip()
            if len(s) < args.min:
                continue
            total += 1
            for cat, rx in COMPILED.items():
                if rx.search(s):
                    if len(cat_hits[cat]) < args.max_hits:
                        cat_hits[cat].append((m.start(), s))
                    break
        out.write(f"file: {args.path}\nbytes: {len(data)}\nstrings>={args.min}: {total}\n")
        for cat, hits in cat_hits.items():
            out.write(f"\n== {cat} ({len(hits)}) ==\n")
            for off, s in hits:
                out.write(f"  0x{off:08x}  {s}\n")
    finally:
        if out is not sys.stdout:
            out.close()


if __name__ == "__main__":
    main()
