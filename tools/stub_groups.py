#!/usr/bin/env python3
"""stub_groups.py - group the SDK stub list of the Ghidra draft config by library/prefix.

Reads the `stubs = [...]` array of recomp/cn_config_ghidra_draft.toml (ExportPS2Functions output) and
prints the name@address entries grouped by Sony SDK library, plus the untracked_stubs count.

Usage:
    python tools/stub_groups.py [--draft PATH] [--from CSV]
"""
import argparse
import collections
import re
from pathlib import Path

# Sony PS2 SDK library prefixes -> library (docs/GAME.md cross-reference).
LIBRARIES = [
    ("sceGs", "libgraph/libgpu (GS)"),
    ("sceDma", "libdma"),
    ("scePad", "libpad (incl. multitap)"),
    ("sceMtap", "libmtap (multitap)"),
    ("sceSd", "libsdr (sound)"),
    ("sceCd", "libcdvd (CD/DVD)"),
    ("sceMc", "libmc (memory card)"),
    ("sceMpeg", "libmpeg (IPU/MPEG)"),
    ("sceIpu", "libipu"),
    ("sceSif", "sif (IOP RPC)"),
    ("sceS", "s kernel/EE"),
    ("sceVu", "VU/kernel"),
    ("sceFs", "fileio (IOP)"),
    ("sceC", "lib/c runtime"),
    ("SetGsCrt", "libgraph misc"),
    ("ResetEE", "kernel misc"),
]


def library_of(name):
    for prefix, library in LIBRARIES:
        if name.startswith(prefix):
            return library
    return "(other)"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--draft", default=str(Path(__file__).resolve().parent.parent / "recomp" / "cn_config_ghidra_draft.toml"))
    a = ap.parse_args()

    draft = Path(a.draft).read_text(encoding="utf-8", errors="replace")

    start = draft.index("stubs = [")
    end = draft.index("]", start)
    block = draft[start:end]
    stubs = re.findall(r'"([^"]+@0x[0-9A-Fa-f]+)"', block)
    groups = collections.defaultdict(list)
    for entry in stubs:
        name, _, addr = entry.partition("@")
        groups[library_of(name)].append((name, addr))

    untracked = re.search(r"untracked_stub_count\s*=\s*(\d+)", draft)
    m = re.search(r"\[ghidra_export\](.*?)(\[|\Z)", draft, re.S)
    summary = m.group(1).strip() if m else ""

    print(f"stubs nomeados: {len(stubs)}; untracked: {untracked.group(1) if untracked else '?'}")
    if summary:
        print(summary)
    for library in sorted(groups):
        print(f"\n== {library} ({len(groups[library])}) ==")
        for name, addr in sorted(groups[library]):
            print(f"  {addr}  {name}")


if __name__ == "__main__":
    main()
