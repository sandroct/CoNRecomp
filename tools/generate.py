#!/usr/bin/env python3
"""generate.py - regenerate the recompiled C++ from your own Champions of Norrath (USA) disc image.

    1. Finds the disc image (game/*.iso, CON_ISO or --iso) and reads SYSTEM.CNF to get the boot ELF name.
    2. Extracts the ELF into build/extracted/ and checks its SHA-256 against the expected hash.
    3. Stages recomp/cn.toml in build/ with absolute paths (input -> extracted ELF, output -> generated/).
    4. Runs ps2_recomp (tools/ps2_recomp.exe, or built with -DCON_BUILD_RECOMP=ON) into generated/ via a
       staging directory, so unchanged files keep their timestamps.

Expected hash: 9A60F13C5C316D619A288821CC9496CC61864106B7872B52EE141CAED3756FF0 (SLUS-20565, USA).
Generic tool; nothing here is taken from another port's generated output.
"""
import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "port"
STAGING_BUILD = ROOT / "build" / "staging"
EXTRACTED = ROOT / "build" / "extracted"
GENERATED = ROOT / "generated"
RECOMP = ROOT / "tools" / "ps2_recomp.exe"
BUILT_RECOMP = BUILD / "runtime" / "ps2xRecomp" / "ps2_recomp.exe"
WORKERS = os.environ.get("CON_WORKERS", "4")
SHA256 = "9a60f13c5c316d619a288821cc9496cc61864106b7872b52ee141caed3756ff0"


def run(cmd, **kw):
    print("+", " ".join(str(c) for c in cmd), flush=True)
    return subprocess.run([str(c) for c in cmd], **kw).returncode


def find_iso(arg=None):
    if arg:
        return Path(arg)
    env = os.environ.get("CON_ISO")
    if env:
        return Path(env)
    for p in sorted((ROOT / "game").glob("*.iso")):
        return p
    sys.exit("No disc image found. Put your own Champions of Norrath (USA) image in game/ or pass --iso <path>.")


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def boot_elf_name(cnf_text):
    # BOOT2 = cdrom0:\SLUS_205.65;1
    m = re.search(r"boot2\s*=\s*cdrom0:\\(.+?);1", cnf_text, re.IGNORECASE)
    if not m:
        sys.exit("no BOOT2 entry in SYSTEM.CNF")
    return m.group(1)


def extract(iso, elf_name):
    EXTRACTED.mkdir(parents=True, exist_ok=True)
    # read SYSTEM.CNF (small, first directory entry)
    out = run([sys.executable, ROOT / "tools" / "iso_extract.py", iso, "--out", EXTRACTED, "--match", "SYSTEM.CNF"])
    if out:
        sys.exit("iso_extract failed for SYSTEM.CNF")
    cnf = (EXTRACTED / "SYSTEM.CNF").read_text(errors="replace")
    elf = boot_elf_name(cnf)
    if elf != elf_name:
        print(f"note: SYSTEM.CNF boot ELF is {elf} (expected {elf_name})")
    if (EXTRACTED / elf).exists():
        print(f"{elf} already extracted")
    else:
        out = run([sys.executable, ROOT / "tools" / "iso_extract.py", iso, "--out", EXTRACTED, "--match", elf])
        if out:
            sys.exit("iso_extract failed for the boot ELF")
    target = EXTRACTED / elf
    digest = sha256_file(target)
    if digest != SHA256:
        sys.exit(f"SHA-256 mismatch for {elf}: {digest} (expected {SHA256}). "
                 "Different disc revision: check docs/GAME.md before continuing.")
    print(f"OK: {elf} sha256 {digest}")
    return target


def stage_toml(elf_path):
    toml = (ROOT / "recomp" / "cn.toml").read_text()
    toml = re.sub(r'(?m)^input\s*=.*$', f'input = "{elf_path.as_posix()}"', toml)
    toml = re.sub(r'(?m)^output\s*=.*$', f'output = "{GENERATED.as_posix()}"', toml)
    toml = re.sub(r'(?m)^ghidra_output\s*=.*$', f'ghidra_output = "{(ROOT / "recomp" / "cn_functions.csv").as_posix()}"', toml)
    staged = ROOT / "build" / "cn.staging.toml"
    staged.parent.mkdir(parents=True, exist_ok=True)
    staged.write_text(toml)
    return staged


def generate(iso_path):
    elf = extract(iso_path, "SLUS_205.65")
    RECOMP.parent.mkdir(parents=True, exist_ok=True)
    recomp = RECOMP if RECOMP.exists() else BUILT_RECOMP
    if not recomp.exists():
        print("recompiler not found: tools/ps2_recomp.exe (or build it with -DCON_BUILD_RECOMP=ON)")
        return 1
    staged = stage_toml(elf)
    out = run([recomp, staged])
    if out:
        sys.exit(f"ps2_recomp failed ({out})")
    files = list(GENERATED.glob("*.cpp"))
    total = sum(f.stat().st_size for f in files)
    print(f"\n{len(files)} .cpp files in generated/ ({total / (1 << 30):.2f} GB)")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--iso", default="")
    a = ap.parse_args()
    return generate(find_iso(a.iso))


if __name__ == "__main__":
    sys.exit(main())
