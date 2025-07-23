from setuptools import setup, Extension
import os
from pathlib import Path


if os.environ.get("CC") == "icx":
    lib = []
    lib_dirs = [str(Path(os.environ.get("ONEAPI_ROOT"), "compilier", "latest", "lib"))]
    args = ["-fortlib"]
else:
    lib = ["gfortran"]
    lib_dirs = []
    args = [
        "-ggdb",
        "-O",
        "-fno-eliminate-unused-debug-symbols",
        "-fvar-tracking-assignments",
    ]


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
            py_limited_api=True,
        )
    ],
)
