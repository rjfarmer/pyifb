# SPDX-License-Identifier: GPL-2.0+

import pyifb as p
import pyifb.utils as utils

import pytest
import ctypes
from pathlib import Path


lib = ctypes.CDLL(Path("tests", f"bindc.{utils.library_ext()}"))


class TestF90:
    def test_make(self):
        a = p.CFI_cdesc()

        # for i in range(0,6):
        #     a.in_dll(lib, f'a_int_{i}d')
        #     assert a.rank == i
