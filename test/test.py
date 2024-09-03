#!/usr/bin/env python3

from argparse import ArgumentParser
from contextlib import chdir
from pathlib import Path
from subprocess import run
from sys import stderr


TEST_FILE = "test.list"


def main() -> int:
    parser = ArgumentParser(description="Test script")

    parser.add_argument("target", nargs="?", default="all")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--rebuild", action="store_true")
    parser.add_argument("--release", action="store_true")

    if not Path(TEST_FILE).exists():
        print(f"File not found: {TEST_FILE}", file=stderr)
        return 1

    args = parser.parse_args()
    print(f"args: {vars(args)}\n")

    build_type = "release" if args.release else "debug"
    preset = f"conan-{build_type}"
    target = args.target

    if not args.no_build or args.rebuild:
        if not Path("build", build_type.capitalize(), "CMakeCache.txt").exists():
            cmake_gen = ["cmake", "--preset", preset]
            run(cmake_gen).check_returncode()

        cmake_build = ["cmake", "--build", "--preset", preset, "--target", target]
        if args.rebuild:
            cmake_build.append("--clean-first")
        run(cmake_build).check_returncode()

        print()

    binary_path = Path("build") / build_type.capitalize() / "main"

    with open(TEST_FILE) as f:
        run([str(binary_path)], input=f.read(), text=True).check_returncode()

    return 0


if __name__ == "__main__":
    with chdir(Path(__file__).absolute().parent):
        ret = main()
        exit(ret)
