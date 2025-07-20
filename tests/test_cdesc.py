# SPDX-License-Identifier: GPL-2.0+

import pyifb as p

import pytest


class TestCdescT:
    def test_members(self):
        cdesc = p.CFI_cdesc_t()

        assert cdesc.attribute == 0
        assert cdesc.base_addr == 0
        assert cdesc.elem_len == 0
        assert cdesc.rank == 0
        assert cdesc.type == 0
        assert cdesc.version == 0
        assert cdesc.dim is None
