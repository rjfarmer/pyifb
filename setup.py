from setuptools import setup, Extension
import os
from pathlib import Path
import platform
from typing import Any

lib: list[Any] = []
lib_dirs: list[Any] = []
args: list[Any] = []
link_args: list[Any] = []
extra_objects: list[Any] = []


def _detect_clang_fortran_runtime():
    """Return (libraries, library_dirs) for the available Fortran runtime."""
    search_dirs = []

    # Fedora flang runtime lives under a clang-versioned directory.
    clang_root = Path("/usr/lib/clang")
    if clang_root.exists():
        for path in sorted(clang_root.glob("*/lib/*"), reverse=True):
            if path.is_dir():
                search_dirs.append(path)

    # Common distro fallback locations.
    search_dirs.extend(
        [
            Path("/usr/lib/llvm-18/lib"),
            Path("/usr/lib/llvm-19/lib"),
            Path("/usr/lib/llvm-20/lib"),
            Path("/usr/lib64"),
            Path("/usr/lib"),
        ]
    )

    candidates = [
        ("flang_rt.runtime", "libflang_rt.runtime.so"),
        ("flang_rt.runtime", "libflang_rt.runtime.a"),
        ("FortranRuntime", "libFortranRuntime.so"),
        ("FortranRuntime", "libFortranRuntime.a"),
    ]

    seen = set()
    for lib_dir in search_dirs:
        lib_dir = lib_dir.resolve()
        if lib_dir in seen or not lib_dir.exists():
            continue
        seen.add(lib_dir)

        for lib_name, file_name in candidates:
            if (lib_dir / file_name).exists():
                return [lib_name], [str(lib_dir)]

    # Preserve previous behavior if runtime files are not discoverable.
    return ["FortranRuntime"], []


if os.environ.get("CC") == "icx":
    oneapi_root = os.environ.get("ONEAPI_ROOT")
    if oneapi_root is not None:
        lib_dirs = [str(Path(oneapi_root, "compilier", "latest", "lib"))]
    args = ["-fortlib", "-O2"]
    link_args = ["-fortlib"]
elif os.environ.get("CC") == "clang":
    lib, lib_dirs = _detect_clang_fortran_runtime()


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
            libraries=lib,
            extra_compile_args=args,
            extra_link_args=link_args,
            extra_objects=extra_objects,
            py_limited_api=True,
        )
    ],
)
