# SPDX-License-Identifier: GPL-2.0+

import os
import shlex
import shutil
import subprocess
from pathlib import Path

import pytest


def _cmake_generator_args():
    if shutil.which("ninja") is not None:
        return ["-G", "Ninja"]
    return []


def _run(command, cwd, env):
    completed = subprocess.run(command, cwd=cwd, env=env, check=False)
    if completed.returncode != 0:
        raise pytest.UsageError(
            f"Command failed ({completed.returncode}): {' '.join(command)}"
        )


def pytest_configure(config):
    _ = config
    CC = os.environ.get("CC", "gcc")
    FC = os.environ.get("FC", "gfortran")

    # Validate compiler executables up front for a clearer failure message.
    cc_exe = shlex.split(CC)[0]
    fc_exe = shlex.split(FC)[0]
    if shutil.which(cc_exe) is None:
        raise pytest.UsageError(f"C compiler not found: CC={CC!r}")
    if shutil.which(fc_exe) is None:
        raise pytest.UsageError(f"Fortran compiler not found: FC={FC!r}")
    if shutil.which("cmake") is None:
        raise pytest.UsageError("CMake executable not found")

    tests_dir = Path(__file__).resolve().parent
    build_dir = tests_dir / ".cmake-build"
    build_dir.mkdir(exist_ok=True)

    env = {**os.environ, "CC": CC, "FC": FC}

    _run(
        [
            "cmake",
            "-S",
            str(tests_dir),
            "-B",
            str(build_dir),
            *(_cmake_generator_args()),
        ],
        cwd=tests_dir,
        env=env,
    )
    _run(
        ["cmake", "--build", str(build_dir), "--target", "clean"],
        cwd=tests_dir,
        env=env,
    )
    _run(["cmake", "--build", str(build_dir)], cwd=tests_dir, env=env)
