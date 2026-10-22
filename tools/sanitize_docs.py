#!/usr/bin/env python3
"""sanitize_docs.py - replace machine-local paths in docs with generic descriptions.

Prepared for the public repository: no local paths (usernames, local clone locations) stay in the
committed documentation. Plain string replacement, safe to re-run.

Usage: python tools/sanitize_docs.py [--dry-run]
"""
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent / "docs"

# (local path, generic replacement) - applied to the text of docs/*.md and docs/*.txt
PAIRS = [
    ("C:\\Users\\Alessandro\\AppData\\Local\\Programs\\Python\\Python312\\python.exe", "python (3.10+)"),
    ("C:\\Users\\Alessandro\\dbz-bt4-recomp", "the recomp-code/dbz-bt4-recomp repository (GPL-3.0)"),
    ("C:\\Users\\Alessandro\\open-champions-godot", "the Open-Champions/open-champions-godot repository (MIT)"),
    ("C:\\Users\\Alessandro\\ps2-recomp-Agent-SKILL", "the hkmodd/ps2-recomp-Agent-SKILL repository"),
    ("C:\\Users\\Alessandro\\PS2Recomp", "your ran-j/PS2Recomp clone"),
    ("C:\\Users\\Alessandro\\Decomp\\slus-20565", "your Ghidra analysis of the SLUS-20565 ELF"),
    ("C:\\recomp\\CoNRecomp", "<this repository>"),
    ("C:\\recomp", "<parent>"),
    ("C:\\Users\\Alessandro\\Desktop\\CoNRecomp_HANDOFF.txt", "docs/HANDOFF.txt (in the repository)"),
    ("C:\\Users\\Alessandro\\Desktop", "your Desktop"),
    ("C:\\Users\\Alessandro\\cn-recomp", "obsolete copies folder (not used)"),
    ("C:\\Users\\Alessandro", "<user>"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP\\Ferramentas \u00dateis\\pcsx2-v2.9.131-windows-x64-Qt\\ISO E BIOS\\Champions of Norrath (USA).iso",
     "your own Champions of Norrath (USA) disc image"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP\\Ferramentas \u00dateis\\pcsx2-v2.9.131-windows-x64-Qt\\ISO E BIOS",
     "your own disc image folder"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP\\Ferramentas \u00dateis\\UrbanReignPC-1.3.0\\game",
     "a compiled Urban Reign PC release (reference only, nothing copied)"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP\\Ferramentas \u00dateis\\UrbanReignPC-1.3.0",
     "the Urban Reign PC release (reference only)"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP\\Ferramentas \u00dateis\\urban-reign-recomp-main",
     "the urban-reign-recomp source tree (GPL-3.0)"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP\\Ferramentas \u00dateis",
     "<tools>"),
    ("C:\\PROJETOS\\Champions Recomp for PC - PS2RECOMP",
     "<project-root>"),
]


def main():
    dry = "--dry-run" in sys.argv
    files = sorted(p for p in ROOT.rglob("*") if p.suffix in (".md", ".txt") and p.is_file())
    changed = 0
    for p in files:
        text = original = p.read_text(encoding="utf-8")
        for local, generic in PAIRS:
            text = text.replace(local, generic)
        if text != original:
            changed += 1
            print(("would change " if dry else "changed  ") + str(p.relative_to(ROOT.parent)))
            if not dry:
                p.write_text(text, encoding="utf-8")
    print(f"{changed} arquivo(s)" + (" (dry-run, nada escrito)" if dry else " sanitizados"))


if __name__ == "__main__":
    main()
