import os
import shutil
import subprocess  # nosec B404
from itertools import product
from pathlib import Path
from typing import Any

from setuptools import Extension, setup

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
    )  # nosec B603
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
    # Use gcc/gfortran header and library as clang's iso-fortran_binding.h
    # does not seem consistent with fedora libflang_runtime.so (type and attribute fields are flipped)
    # and i cant seem to get the include and link flags right .
    include_dirs = [_get_command_output(["gfortran", "-print-file-name=include"])]
    lib = ["gfortran"]
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
    options={"bdist_wheel": {"py_limited_api": "cp310"}},
)
