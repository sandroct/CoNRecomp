#!/usr/bin/env python3
"""finalize_build.py - prepara o link final do build distribuido.

1. Lista os alvos .cpp.obj ainda sem .obj no disco (lists/missing.txt).
2. Se houver faltantes, anexa seus nomes em CON_NOOPT_SOURCES (src/CMakeLists.txt) para
   compilarem com /Od (erros de heap C1060/C1002) - so as regras desses arquivos mudam.
3. Nao compila: use depois
     tools\\win\\vsenv.cmd cmake -S . -B build\\port
     python tools\\build_subset.py lists\\missing.txt --jobs 8
"""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "port"
NINJA = r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"


def main():
    out = subprocess.run([NINJA, "-C", str(BUILD), "-t", "targets", "all"],
                         capture_output=True, text=True)
    objs = [l.split(":", 1)[0].strip() for l in out.stdout.splitlines() if l.strip().endswith(".cpp.obj")]
    missing = [o for o in objs if not pathlib.Path(BUILD, o).exists()]
    if not missing:
        print("nada faltando: pronto para biblioteca + link")
        return
    pathlib.Path(ROOT / "lists").mkdir(exist_ok=True)
    (ROOT / "lists" / "missing.txt").write_text("\n".join(missing) + "\n", encoding="utf-8")

    names = [o.replace("\\", "/").split("/")[-1].replace(".cpp.obj", ".cpp") for o in missing]
    cm = ROOT / "src" / "CMakeLists.txt"
    txt = cm.read_text(encoding="utf-8")
    m = re.search(r'set\(CON_NOOPT_SOURCES "([^"]*)"', txt)
    current = m.group(1)
    added = [n for n in names if n not in current.split(";")]
    if added:
        new = current + (";" if current else "") + ";".join(added)
        txt = txt[:m.start(1)] + new + txt[m.end(1):]
        cm.write_text(txt, encoding="utf-8")
    print(f"faltantes: {len(missing)} | novos em CON_NOOPT_SOURCES: {len(added)} | lista: lists/missing.txt")


if __name__ == "__main__":
    sys.exit(main())
