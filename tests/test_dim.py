# SPDX-License-Identifier: GPL-2.0+

import pyifb as p

import pytest


class TestDimT:
    def test_members(self):
        dim = p.CFI_dim_t()

        assert dim.extent == 0
        assert dim.lower_bound == 0
        assert dim.sm == 0

    def test_set(self):
        dim = p.CFI_dim_t()

        dim.extent = 1
        dim.lower_bound = 1
        dim.sm = 1

        assert dim.extent == 1
        assert dim.lower_bound == 1
        assert dim.sm == 1
