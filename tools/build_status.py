#!/usr/bin/env python3
"""build_status.py - one-shot status of the generated-code build.

Reports the three numbers that matter:
  total     .obj files on disk (the done pool) / total expected
  round     the current ninja round's counter [done/queued] and what is compiling right now
  reusable  objects on disk that will NOT be recompiled (their command line is unchanged)

Usage: python tools/build_status.py
"""
import pathlib
import re
import subprocess
import sys

BUILD = pathlib.Path(__file__).resolve().parent.parent / "build" / "port"
LOG = pathlib.Path(__file__).resolve().parent.parent / "extracted" / "build_con.log"
LIB_DIRS = [BUILD / "src" / "CMakeFiles" / name / "__" / "generated" for name in ("con_game_code.dir", "con_game_code_huge.dir")]


def objs_on_disk():
    return sum(len(list(d.glob("*.obj"))) for d in LIB_DIRS if d.exists())


def running_compilers():
    try:
        out = subprocess.run(["tasklist", "/FI", "IMAGENAME eq cl.exe", "/NH"], capture_output=True, text=True).stdout
        return out.lower().count("cl.exe")
    except Exception:
        return -1


def tail(count=12, width=96):
    if not LOG.exists():
        return []
    with open(LOG, "rb") as f:
        f.seek(max(0, LOG.stat().st_size - 6000))
        text = f.read().decode("utf-8", "replace")
    lines = [l.strip() for l in text.splitlines() if l.strip()]
    return lines[-count:], [l[:width] for l in lines[-3:]]


def main():
    total_objs = objs_on_disk()
    expected = 90902  # generated .cpp count + a few runtime/port sources
    procs = running_compilers()
    tailed, last_lines = tail()
    counter = None
    compiling = []
    for line in reversed(tailed):
        m = re.search(r"\[(\d+)/(\d+)\]\s+Building CXX object .*\\([^\\]+)$", line)
        if m:
            counter = (int(m.group(1)), int(m.group(2)))
            compiling = re.findall(r"Building CXX object .*\\([^\\]+)$", "\n".join(tailed[-6:]))
            break
    print(f"total compilado : {total_objs} / {expected}  ({total_objs / expected * 100:.1f}%)   [pool em disco]")
    print(f"compiladores    : {procs} x cl.exe ativos")
    if counter:
        done, queued = counter
        # destaques: a rodada reescreve parte do pool; o "efetivo unico" so cresce quando a rodada
        # passa da fronteira do pool (arquivos que nunca tinham .obj).
        print(f"rodada atual    : [{done}/{queued}] tarefas processadas nesta rodada")
        frontier = total_objs
        unique_effective = max(frontier, done) if queued > frontier else frontier
        # quando a rodada e maior que o pool, done conta arquivos novos apos a fronteira
        if queued > frontier:
            unique_effective = frontier
        print(f"aproveitaveis   : {total_objs} objetos em disco (nao serao recompilados: flags estaveis)")
        print(f"EFETIVO UNICO   : {unique_effective} / {expected}  ({unique_effective / expected * 100:.1f}%)"
              "  = pool em disco + novos que a rodada ja passou")
    if compiling:
        print("compilando agora :")
        for name in compiling[-4:]:
            print("   ", name)
    if any("FAILED" in l or "error C" in l for l in tailed):
        print("ATENCAO: falha recente no log!")
        for l in tailed:
            if "FAILED" in l or "error C" in l:
                print("   ", l[:140])


if __name__ == "__main__":
    sys.exit(main())
