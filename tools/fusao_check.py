#!/usr/bin/env python3
"""fusao_check.py - audita se o build final esta reaproveitando (ou refazendo) o trabalho.

Classifica cada .obj compilado durante o build final (extracted/build_final.log):
  - partA        : estava na nossa metade A (ja compilado por nos) -> REFAZENDO = desperdicio
  - partB        : estava na metade do irmao (objeto copiado do pendrive) -> REFAZENDO = desperdicio
  - novo         : nao estava em lista nenhuma nem no ninja_log -> falta real, compilar e legitimo
Uso: python tools/fusao_check.py
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "port"
OBJDIR = BUILD / "src" / "CMakeFiles" / "con_game_code.dir" / "__" / "generated"


def names(path):
    return set(pathlib.Path(l).name for l in pathlib.Path(path).read_text(encoding="utf-8").splitlines() if l.strip())


def main():
    a = names(ROOT / "lists" / "partA.txt")
    b = names(ROOT / "lists" / "partB.txt")
    log = BUILD / ".ninja_log"
    compiled_before = set()
    if log.exists():
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            parts = line.split("\t")
            if len(parts) >= 4:
                compiled_before.add(pathlib.Path(parts[3]).name)

    flog = (ROOT / "extracted" / "build_final.log").read_text(errors="replace")
    done = set(re.findall(r"Building CXX object \S*?__\\generated\\([A-Za-z0-9_.]+\.cpp\.obj)", flog))
    # fallback: qualquer linha com o caminho completo
    if not done:
        done = set(pathlib.Path(l).name for l in
                   re.findall(r"Building CXX object (\S+\.obj)", flog))

    in_a = done & a
    in_b = done & b
    novo = done - a - b - compiled_before
    rework_known = (done & compiled_before) - novo
    print(f"compilados no build final: {len(done)}")
    print(f"  da nossa metade A (refectira!): {len(in_a)}")
    print(f"  da metade B do irmao (refectira!): {len(in_b)}")
    print(f"  ja tinham registro no ninja_log (recompilados!): {len(rework_known)}")
    print(f"  NOVOS (faltavam de verdade): {len(novo)}")
    if in_b:
        print("amostra B refeita:", sorted(in_b)[:5])
    if in_a:
        print("amostra A refeita:", sorted(in_a)[:5])


if __name__ == "__main__":
    sys.exit(main())
