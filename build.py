"""CoNRecomp: build and run (Champions of Norrath, USA SLUS-20565).

    python build.py              numbered menu
    python build.py build        configure (first time) and compile con.exe
    python build.py play [args]  run con.exe (extra arguments go to it)
    python build.py generate     recompile the game code from your disc image in game/
    python build.py configure    run CMake again
    python build.py bench        measure frames/s on a savestate from bench/states

Needs Python 3.10+, Visual Studio 2022 with C++ (cl, CMake and Ninja come with it; set VSROOT if it is not in the
default folder) and your own Champions of Norrath (USA) disc image in game/.

Structure follows the urban-reign-recomp project (GPL-3.0); the game-driven values (hashes, function map, config)
are built from Champions of Norrath analysis, not copied from Urban Reign.
"""
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BUILD = ROOT / "build" / "port"
BIN = BUILD / "bin"
VSENV = ROOT / "tools" / "win" / "vsenv.cmd"
JOBS = os.environ.get("CON_JOBS", "8")


def run(cmd, **kwargs):
    print("+", " ".join(str(c) for c in cmd), flush=True)
    return subprocess.run([str(c) for c in cmd], **kwargs).returncode


def in_vs(*cmd):
    """Runs a command inside the Visual Studio x64 environment (tools/win/vsenv.cmd)."""
    return run(["cmd", "/c", VSENV, *cmd])


def game_running():
    out = subprocess.run(["tasklist", "/FI", "IMAGENAME eq con.exe", "/NH"], capture_output=True, text=True).stdout
    return "con.exe" in out.lower()


def has_generated_code():
    return (ROOT / "generated" / "register_functions.cpp").exists()


def configure(extra=()):
    args = ["cmake", "-S", ROOT, "-B", BUILD, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=RelWithDebInfo"]
    # CON_DEPS: a FetchContent _deps folder already downloaded (raylib-src, ...) to reuse instead of downloading.
    deps = os.environ.get("CON_DEPS")
    if deps:
        for name in ("raylib", "sdl3", "fmt", "elfio", "toml11", "libdwarf", "rabbitizer"):
            src = Path(deps) / f"{name}-src"
            if src.is_dir():
                args.append(f"-DFETCHCONTENT_SOURCE_DIR_{name.upper()}={src}")
    return in_vs(*args, *extra)


def build(target="con"):
    if not has_generated_code():
        print("The recompiled game code is missing: run the 'Generate game code' option first.")
        return 1
    if game_running():
        print("con.exe is running: close the game first (the link would fail).")
        return 1
    if not (BUILD / "CMakeCache.txt").exists() and configure():
        return 1
    return in_vs("cmake", "--build", BUILD, "--target", target, "--", "-j", JOBS)


def play(args=()):
    exe = BIN / "con.exe"
    if not exe.exists():
        print("con.exe not built yet: run the 'Build' option first.")
        return 1
    return run([exe, *args], cwd=BIN)


def generate(args=()):
    # The recompiler: a prebuilt tools/ps2_recomp.exe (not in the repository) or built from runtime/ps2xRecomp.
    built = BUILD / "runtime" / "ps2xRecomp" / "ps2_recomp.exe"
    if not (ROOT / "tools" / "ps2_recomp.exe").exists() and not built.exists():
        print("Building the recompiler (ps2_recomp) first.")
        if configure(["-DCON_BUILD_RECOMP=ON"]) or in_vs("cmake", "--build", BUILD, "--target", "ps2_recomp", "--", "-j", JOBS):
            return 1
    return run([sys.executable, ROOT / "tools" / "generate.py", *args])


def pick(title, options):
    """Numbered choice; returns the index or None (0 = back)."""
    print(f"\n{title}")
    for i, text in enumerate(options, 1):
        print(f"  {i}) {text}")
    print("  0) Back")
    answer = input("> ").strip()
    if answer.isdigit() and 1 <= int(answer) <= len(options):
        return int(answer) - 1
    return None


def bench(args=()):
    if args:
        return run([sys.executable, ROOT / "tools" / "autotest" / "bench_state.py", *args])
    states = sorted((ROOT / "bench" / "states").glob("*.state"))
    if not states:
        print("No savestates in bench/states.")
        return 1
    choice = pick("Savestate to measure:", [s.stem for s in states])
    if choice is None:
        return 0
    return run([sys.executable, ROOT / "tools" / "autotest" / "bench_state.py", "--state", states[choice],
                "--measure", "20"])


def has_disc_image():
    return any(p.suffix.lower() in (".iso", ".bin", ".img") for p in (ROOT / "game").glob("*"))


def open_game_folder():
    if BIN.exists():
        os.startfile(BIN)
    else:
        print("Not built yet.")


# Building from scratch, in order.
STEPS = [
    ("Generate the game code from your disc image (put the .iso in game/ first)", lambda: generate(),
     has_generated_code),
    ("Build con.exe (the first time takes over an hour)", lambda: build(), lambda: (BIN / "con.exe").exists()),
    ("Play", lambda: play(), None),
]
# Everything else.
UTILITIES = [
    ("Build and play", lambda: build() or play()),
    ("Configure again (CMake)", lambda: configure()),
    ("Build only the PCSX2 GS (pcsx2gs.dll)", lambda: build("pcsx2gs")),
    ("Benchmark a savestate (bench/states)", lambda: bench()),
    ("Open the game folder (build/port/bin)", open_game_folder),
]


def menu():
    actions = [action for _, action, _ in STEPS] + [action for _, action in UTILITIES]
    while True:
        print("\n=== CoNRecomp: build ===")
        if not has_disc_image():
            print("  (no disc image in game/ yet: copy your Champions of Norrath (USA) .iso there)")
        for i, (text, _, done) in enumerate(STEPS, 1):
            mark = "  [done]" if done and done() else ""
            print(f"  Step {i}) {text}{mark}")
        print("\n  Utilities")
        for i, (text, _) in enumerate(UTILITIES, len(STEPS) + 1):
            print(f"  {i}) {text}")
        print("\n  0) Exit")
        answer = input("> ").strip()
        if answer in ("0", "q", ""):
            return 0
        if answer.isdigit() and 1 <= int(answer) <= len(actions):
            result = actions[int(answer) - 1]()
            if isinstance(result, int) and result:
                print(f"(exit code {result})")
        else:
            print("Pick a number from the list.")


COMMANDS = {
    "build": lambda a: build(a[0] if a else "con"),
    "play": play,
    "run": play,
    "generate": generate,
    "configure": configure,
    "bench": bench,
}


def main():
    if len(sys.argv) < 2:
        return menu()
    command, args = sys.argv[1], sys.argv[2:]
    if command not in COMMANDS:
        print(__doc__)
        return 2
    return COMMANDS[command](args)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.exit(130)
