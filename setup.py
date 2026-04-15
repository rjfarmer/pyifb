from setuptools import setup, Extension
import os
from pathlib import Path
import shutil
import subprocess
from typing import Any
from itertools import product

lib: list[Any] = []
lib_dirs: list[Any] = []
args: list[Any] = []
link_args: list[Any] = []
extra_objects: list[Any] = []
include_dirs: list[Any] = []


def _get_command_output(command: list[str]) -> str | None:
    completed = subprocess.run(
        command,
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        return None
    return completed.stdout.strip() or None


compiler = os.environ.get("CC", "gcc")

if "icx" in compiler:
    oneapi_root = os.environ.get("ONEAPI_ROOT")
    if oneapi_root is not None:
        lib_dirs = [str(Path(oneapi_root, "compilier", "latest", "lib"))]
    args = ["-fortlib", "-O2"]
    link_args = ["-fortlib"]
elif "clang" in compiler:
    clang = shutil.which(compiler)
    if clang:
        lib_dirs = [_get_command_output([clang, "-print-runtime-dir"])]
        include_dirs = [_get_command_output([clang, "-print-file-name=include"])]

        guess = ["flang_rt.runtime", "FortranRuntime"]
        ext = [".so", ".a", ".dylib"]
        for libname, e in product(guess, ext):
            lib_path = _get_command_output([clang, f"-print-file-name=lib{libname}{e}"])
            if lib_path and os.path.isfile(lib_path):
                lib = [libname]
                break

    args = ["-O2"]

else:
    lib = ["gfortran"]
    args = [
        "-ggdb",
        "-O2",
        "-fno-eliminate-unused-debug-symbols",
        "-fvar-tracking-assignments",
        "-Wall",
        "-fstack-protector-all",
    ]


setup(
    ext_modules=[
        Extension(
            name="pyifb.ifb",
            sources=[
                "pyifb/ifb.c",
            ],
            library_dirs=lib_dirs,
            include_dirs=include_dirs,
            libraries=lib,
            extra_compile_args=args,
            extra_link_args=link_args,
            extra_objects=extra_objects,
            py_limited_api=True,
        )
    ],
)
