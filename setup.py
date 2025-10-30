from setuptools import setup, Extension
import os
from pathlib import Path
import platform


lib = []
lib_dirs = []
args = []
link_args = []
extra_objects = []

if os.environ.get("CC") == "icx":
    lib_dirs = [str(Path(os.environ.get("ONEAPI_ROOT"), "compilier", "latest", "lib"))]
    args = ["-fortlib"]
    link_args = ["-fortlib"]
elif os.environ.get("CC") == "clang":

    # Ubuntu only has static version of libFortranRuntime while Fedora has shared
    if "Ubuntu" in platform.version():
        extra_objects = ["libFortranRuntime.a"]
        lib_dirs = ["-L/usr/lib/llvm-18/lib/"]
    else:
        lib = ["FortranRuntime"]


else:
    lib = ["gfortran"]
    args = [
        "-ggdb",
        "-O",
        "-fno-eliminate-unused-debug-symbols",
        "-fvar-tracking-assignments",
        "-Wall",
        "-fstack-protector-all",
        "-fcheck=all",
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
            extra_link_args=link_args,
            extra_objects=extra_objects,
            py_limited_api=True,
        )
    ],
)
