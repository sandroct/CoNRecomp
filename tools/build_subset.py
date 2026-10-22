#!/usr/bin/env python3
"""build_subset.py - compila apenas os alvos de uma lista (metade do build distribuido).

Uso: python tools/build_subset.py lists/partB.txt [--jobs 12] [--chunk 150]

Chama o ninja em blocos (linha de comando tem limite). Alvos ja compilados sao pulados
pelo proprio ninja (rapido). Pode rodar nos DOIS PCs, cada um com sua metade.
"""
import argparse
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def find_ninja():
    exe = shutil.which("ninja")
    if exe:
        return exe
    for cand in [
        r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe",
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe",
    ]:
        if Path(cand).exists():
            return cand
    sys.exit("ninja nao encontrado: instale o VS2022 com CMake tools ou adicione o ninja ao PATH")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("lista")
    ap.add_argument("--build", default=str(ROOT / "build" / "port"))
    ap.add_argument("--jobs", default="12")
    ap.add_argument("--chunk", type=int, default=150)
    a = ap.parse_args()

    targets = [l.strip() for l in Path(a.lista).read_text(encoding="utf-8").splitlines() if l.strip()]
    print(f"{len(targets)} alvos na lista, blocos de {a.chunk}")
    done = 0
    t0 = time.time()
    fails = 0
    for i in range(0, len(targets), a.chunk):
        chunk = targets[i:i + a.chunk]
        cmd = [find_ninja(), "-C", a.build, "-j", a.jobs, *chunk]
        r = subprocess.run(cmd)
        if r.returncode != 0:
            fails += 1
            print(f"bloco {i//a.chunk + 1}: FALHOU (continuando; alvos ficam para a proxima)")
        done += len(chunk)
        el = time.time() - t0
        rate = done / max(el / 60, 0.01)
        print(f"[{done}/{len(targets)}] {rate:.0f} alvos/min | falhas: {fails}", flush=True)
    print(f"fim: {done} alvos em {(time.time()-t0)/60:.1f} min | falhas: {fails}")
    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
