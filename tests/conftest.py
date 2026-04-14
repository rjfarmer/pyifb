# SPDX-License-Identifier: GPL-2.0+

import subprocess
import pytest
import os
import shlex
import shutil


def pytest_configure(config):
    CC = os.environ.get("CC", "gcc")
    FC = os.environ.get("FC", "gfortran")

    # Validate compiler executables up front for a clearer failure message.
    cc_exe = shlex.split(CC)[0]
    fc_exe = shlex.split(FC)[0]
    if shutil.which(cc_exe) is None:
        raise pytest.UsageError(f"C compiler not found: CC={CC!r}")
    if shutil.which(fc_exe) is None:
        raise pytest.UsageError(f"Fortran compiler not found: FC={FC!r}")

    env = {**os.environ, "CC": CC, "FC": FC}
    subprocess.call(["make", "clean"], cwd="tests", env=env)
    subprocess.call(["make"], cwd="tests", env=env)
