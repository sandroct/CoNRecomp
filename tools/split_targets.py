#!/usr/bin/env python3
"""split_targets.py - divide a lista de objetos pendentes do Ninja em duas metades (build distribuido).

Gera lists/partA.txt e lists/partB.txt com os alvos (obj) que cada PC deve compilar.
Cada alvo e uma linha do "ninja -t targets" (termina com .cpp.obj).

Uso:
  1) neste PC (com build/port configurado):
     python tools/split_targets.py --build build/port --out lists
  2) levar lists/partB.txt no pendrive para o outro PC
  3) em cada PC: python tools/build_subset.py lists/partA.txt   (ou partB)
"""
import argparse
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def find_ninja():
    exe = shutil.which("ninja")
    if exe:
        return exe
    for cand in [
        r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe",
    ]:
        if Path(cand).exists():
            return cand
    sys.exit("ninja nao encontrado: instale o VS2022 com CMake tools ou adicione o ninja ao PATH")


def pending_objects(build_dir):
    # alvos que ainda nao tem .obj no disco
    out = subprocess.run([find_ninja(), "-C", str(build_dir), "-t", "targets", "all"],
                         capture_output=True, text=True)
    objs = []
    for line in out.stdout.splitlines():
        name = line.split(":", 1)[0].strip()
        if name.endswith(".cpp.obj"):
            objs.append(name)
    missing = [t for t in objs if not Path(build_dir, t).exists()]
    return missing


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=str(ROOT / "build" / "port"))
    ap.add_argument("--out", default=str(ROOT / "lists"))
    ap.add_argument("--limit", type=int, default=0, help="apenas os N primeiros (teste)")
    a = ap.parse_args()

    missing = pending_objects(Path(a.build))
    if a.limit:
        missing = missing[:a.limit]
    missing.sort()
    half = (len(missing) + 1) // 2
    part_a, part_b = missing[:half], missing[half:]
    out = Path(a.out)
    out.mkdir(exist_ok=True)
    (out / "partA.txt").write_text("\n".join(part_a) + "\n", encoding="utf-8")
    (out / "partB.txt").write_text("\n".join(part_b) + "\n", encoding="utf-8")
    print(f"pendentes: {len(missing)} | parte A: {len(part_a)} | parte B: {len(part_b)}")
    print(f"listas em {out}/ (partA = Este PC, partB = PC do irmao)")


if __name__ == "__main__":
    sys.exit(main())
