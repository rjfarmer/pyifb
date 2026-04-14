# SPDX-License-Identifier: GPL-2.0+

import subprocess
import pytest
import os


def pytest_configure(config):
    CC = os.environ.get("CC", "gcc")
    FC = os.environ.get("FC", "gfortran")
    env = {**os.environ, "CC": CC, "FC": FC}
    subprocess.call(["make", "clean"], cwd="tests", env=env)
    subprocess.call(["make"], cwd="tests", env=env)
