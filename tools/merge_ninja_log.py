#!/usr/bin/env python3
"""merge_ninja_log.py - mescla o .ninja_log de outro computador no nosso (build distribuido).

Sem isso, o ninja nao reconhece os .obj compilados pelo outro PC (falta a "impressao digital" do
comando no registro local) e recompilaria tudo.

Uso: python tools/merge_ninja_log.py <outro .ninja_log>
"""
import pathlib
import shutil
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
OURS = ROOT / "build" / "port" / ".ninja_log"


def parse(line):
    parts = line.rstrip("\n").split("\t")
    if len(parts) >= 4:
        return parts[3]
    return None


def main():
    if len(sys.argv) < 2:
        sys.exit("uso: python tools/merge_ninja_log.py <caminho do .ninja_log do outro PC>")
    theirs = pathlib.Path(sys.argv[1])
    if not theirs.exists():
        sys.exit(f"nao encontrei {theirs}")

    backup = OURS.with_suffix(".log.ours_bak")
    if not backup.exists():
        shutil.copy2(OURS, backup)

    # o log do outro PC pode ter saidas com caminho ABSOLUTO (ex.: C:/recomp/CoNRecomp/build/port/...)
    # enquanto o nosso e relativo (src/CMakeFiles/...). Converte para relativo antes de mesclar.
    prefix = str(OURS.parent).replace("\\", "/") + "/"

    def convert(line):
        parts = line.rstrip("\n").split("\t")
        if len(parts) >= 4 and parts[3].startswith(prefix):
            parts[3] = parts[3][len(prefix):]
        return "\t".join(parts)

    ours = {}
    for line in OURS.read_text(encoding="utf-8", errors="replace").splitlines():
        out = parse(line)
        if out:
            ours[out] = line

    added = skipped = 0
    for line in theirs.read_text(encoding="utf-8", errors="replace").splitlines():
        line = convert(line)
        out = parse(line)
        if not out:
            continue
        if out in ours:
            skipped += 1
            continue
        ours[out] = line
        added += 1

    with open(OURS, "w", encoding="utf-8") as f:
        for out in ours:
            f.write(ours[out] + "\n")
    print(f"merge: +{added} registros dele (caminhos convertidos p/ relativos), {skipped} ja nossos, total {len(ours)}")


if __name__ == "__main__":
    sys.exit(main())
