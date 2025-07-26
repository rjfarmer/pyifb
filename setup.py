from setuptools import setup, Extension
import os
from pathlib import Path


if os.environ.get("CC") == "icx":
    lib = []
    lib_dirs = [str(Path(os.environ.get("ONEAPI_ROOT"), "compilier", "latest", "lib"))]
    args = ["-fortlib"]
    link_args = ["-fortlib"]
elif os.environ.get("CC") == "clang":
    lib = ["FortranRuntime"]
    lib_dirs = []
    args = ["-rtlib=flang-rt"]
    link_args = []

else:
    lib = ["gfortran"]
    lib_dirs = []
    args = [
        "-ggdb",
        "-O",
        "-fno-eliminate-unused-debug-symbols",
        "-fvar-tracking-assignments",
        "-Wall",
        "-fstack-protector-all",
        "-fcheck=all",
    ]
    link_args = []


setup(
    ext_modules=[
        Extension(
            name="pyifb.ifb",
            sources=[
                "pyifb/ifb.c",
            ],
            library_dirs=lib_dirs,
            libraries=lib,
            extra_compile_args=args,
            extra_link_args=link_args,
            py_limited_api=True,
        )
    ],
)
