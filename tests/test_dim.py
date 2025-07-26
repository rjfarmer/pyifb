# SPDX-License-Identifier: GPL-2.0+

import pyifb as p

import pytest


class TestDimT:
    def test_members(self):
        dim = p.ifb.CFI_dim_t()

        assert dim.extent == 0
        assert dim.lower_bound == 0
        assert dim.sm == 0
