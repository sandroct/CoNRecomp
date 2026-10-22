#!/usr/bin/env python3
"""make_link.py - constroi o comando do link final (con.exe) a partir do build.ninja.

O con_game_code.lib estoura o limite de 4 GB do LIB (90.886 objetos), entao o link recebe
diretamente as 16 partes (extracted/parts/pN.lib) no lugar dela.
Uso: python tools/make_link.py [--out run_final_link.bat]
"""
import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "port"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(ROOT / "extracted" / "run_final_link.bat"))
    a = ap.parse_args()

    lines = (BUILD / "build.ninja").read_text(encoding="utf-8", errors="replace").splitlines()
    start = None
    for i, l in enumerate(lines):
        if l.startswith("build bin\\con.exe:") or l.startswith("build bin/con.exe:"):
            start = i
            break
    if start is None:
        sys.exit("edge do con.exe nao encontrado no build.ninja")

    # entradas: a parte apos o nome da regra
    inputs = lines[start].split(":", 1)[1].split(" ", 1)[1].strip().split()
    objs = [p for p in inputs if p.endswith(".obj")]
    # variaveis do edge (linhas seguintes indentadas)
    libs, flags = [], ""
    j = start + 1
    while j < len(lines) and lines[j].startswith("  "):
        m = re.match(r"\s*(\w+) = (.*)", lines[j])
        if m:
            key, val = m.group(1), m.group(2)
            if key == "LINK_LIBRARIES":
                libs = val.split()
            elif key == "LINK_FLAGS":
                flags = val
        j += 1

    parts = sorted((ROOT / "extracted" / "parts").glob("p*.lib"))
    if not parts:
        sys.exit("partes nao encontradas em extracted/parts (rode run_lib_parts16 antes)")
    final_libs = [p for p in libs if p.replace("/", "\\") != r"src\con_game_code.lib"]
    all_inputs = [p for p in libs if p.replace("/", "\\") == r"src\con_game_code.lib"]

    cmd = ["link.exe", "/nologo"]
    cmd += flags.split()
    cmd.append("/INCREMENTAL:NO")  # link nao-incremental: /LTCG sobre 90k objetos levaria horas
    cmd.append("/subsystem:console")
    cmd.append("/FORCE:MULTIPLE")
    cmd.append("/STACK:16777216")
    cmd.append("/OUT:bin\\con.exe")
    cmd.append("/PDB:bin\\con.pdb")
    cmd += objs                       # main.cpp.obj, port/*.obj
    cmd += [str(p) for p in parts]    # as 16 partes (caminhos absolutos)
    cmd += final_libs                 # runtime, iop, pcsx2_gs, ffmpeg, raylib, SDL3, imgui...

    out = ROOT / "extracted" / "run_final_link.bat"
    bat = "@echo off\n" + "cd /d " + str(BUILD) + "\n" + \
          'call "' + str(ROOT / "tools" / "win" / "vsenv.cmd") + '" ' + \
          " ".join('"' + c + '"' if " " in c else c for c in cmd) + "\n" + \
          "echo LINK_EXIT=%ERRORLEVEL%\n"
    out.write_text(bat, encoding="utf-8")
    print(f"comando de link escrito em {out}")
    print(f"  objs: {len(objs)} | partes: {len(parts)} | libs: {len(final_libs)}")
    missing = [l for l in final_libs + all_inputs if not (BUILD / l).exists()]
    if missing:
        print("  ATENCAO, arquivos ausentes:", missing)


if __name__ == "__main__":
    sys.exit(main())
