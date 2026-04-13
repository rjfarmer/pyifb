# SPDX-License-Identifier: GPL-2.0+

import subprocess
import pytest
import _pytest.skipping
import os


def pytest_configure(config):
    subprocess.call(
        [f"CC={os.environ.get('CC')}", "make", "clean"], shell=True, cwd="tests"
    )
    subprocess.call([f"CC={os.environ.get('CC')}", "make"], shell=True, cwd="tests")
