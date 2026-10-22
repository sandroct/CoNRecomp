#!/usr/bin/env python3
"""hle_coverage.py - cross the game's stub list with the runtime's HLE handlers.

For milestone M1 (HLE by address): the Ghidra draft names the game's SDK functions (name@address); the
runtime declares handlers in runtime/ps2xRuntime/include/ps2_call_list.h (PS2_STUB_LIST). This tool
reports which stubs the runtime can bind by name (bindAddressHandler) and which need written handlers.

Usage: python tools/hle_coverage.py [--csv]
"""
import argparse
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parent.parent
CALL_LIST = ROOT / "runtime" / "ps2xRuntime" / "include" / "ps2_call_list.h"
DRAFT = ROOT / "recomp" / "cn_config_ghidra_draft.toml"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", action="store_true", help="machine-readable output")
    a = ap.parse_args()

    text = CALL_LIST.read_text(encoding="utf-8", errors="replace")
    handlers = set(re.findall(r"X\((\w+)\)", text))

    draft = DRAFT.read_text(encoding="utf-8", errors="replace")
    start = draft.index("stubs = [")
    end = draft.index("]", start)
    entries = re.findall(r'"([^"]+@0x[0-9A-Fa-f]+)"', draft[start:end])
    pairs = []
    for e in entries:
        name, _, addr = e.partition("@")
        pairs.append((name, addr))

    covered = [(n, ad) for n, ad in pairs if n in handlers]
    missing = [(n, ad) for n, ad in pairs if n not in handlers]

    if a.csv:
        print("name,address,covered")
        for n, ad in covered:
            print(f"{n},{ad},yes")
        for n, ad in missing:
            print(f"{n},{ad},no")
        return

    print(f"handlers no runtime (ps2_call_list.h): {len(handlers)}")
    print(f"stubs do CoN: {len(pairs)} ({len(set(n for n, _ in pairs))} nomes unicos)")
    print(f"COBERTOS por nome (bindAddressHandler): {len(covered)}")
    print(f"sem handler no runtime: {len(missing)}")
    print("\namostra cobertos:")
    for n, ad in covered[:12]:
        print(f"  {ad}  {n}")
    print("\namostra sem handler:")
    for n, ad in missing[:15]:
        print(f"  {ad}  {n}")


if __name__ == "__main__":
    main()
