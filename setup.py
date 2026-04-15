from setuptools import setup, Extension
import os
from pathlib import Path
import shutil
import subprocess
from typing import Any

lib: list[Any] = []
lib_dirs: list[Any] = []
args: list[Any] = []
link_args: list[Any] = []
extra_objects: list[Any] = []
include_dirs: list[Any] = []


def _append_existing_dirs(search_dirs: list[Path], *dirs: str | Path | None) -> None:
    for directory in dirs:
        if not directory:
            continue
        path = Path(directory).expanduser()
        if path.is_dir():
            search_dirs.append(path)


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


def _append_resource_runtime_dirs(
    search_dirs: list[Path], compiler: str | None
) -> None:
    if not compiler:
        return

    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        return

    resource_dir = _get_command_output([compiler_path, "-print-resource-dir"])
    if resource_dir is None:
        return

    arch = _get_command_output(["uname", "-m"])
    _append_existing_dirs(
        search_dirs,
        Path(resource_dir, "lib", f"{arch}-redhat-linux-gnu") if arch else None,
        Path(resource_dir, "lib", "linux"),
        Path(resource_dir, "lib"),
    )


def _detect_clang_fortran_runtime():
    """Return (libraries, library_dirs) for the available Fortran runtime."""
    search_dirs: list[Path] = []

    flang_rt_dir = os.environ.get("FLANG_RT_DIR")
    if flang_rt_dir:
        _append_existing_dirs(search_dirs, flang_rt_dir)

    llvm_config = shutil.which("llvm-config")
    if llvm_config is not None:
        llvm_libdir = _get_command_output([llvm_config, "--libdir"])
        llvm_prefix = _get_command_output([llvm_config, "--prefix"])
        _append_existing_dirs(
            search_dirs,
            llvm_libdir,
            Path(llvm_prefix, "lib") if llvm_prefix else None,
            Path(llvm_prefix, "lib64") if llvm_prefix else None,
        )

    _append_resource_runtime_dirs(search_dirs, os.environ.get("FC", "flang"))
    _append_resource_runtime_dirs(search_dirs, os.environ.get("CC", "clang"))

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
            Path("/usr/lib/llvm-21/lib"),
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
            include_dirs=include_dirs,
            libraries=lib,
            extra_compile_args=args,
            extra_link_args=link_args,
            extra_objects=extra_objects,
            py_limited_api=True,
        )
    ],
)
