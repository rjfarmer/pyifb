# SPDX-License-Identifier: GPL-2.0+

import pyifb as p

import pytest


class TestCdescT:
    def test_new(self):
        _ = p.ifb.CFI_cdesc_t()

        for i in range(0, p.ifb.CFI_MAX_RANK):
            _ = p.ifb.CFI_cdesc_t(i)

        # assert cdesc.attribute == 0
        # assert cdesc.base_addr == 0
        # assert cdesc.elem_len == 0
        # assert cdesc.rank == 0
        # assert cdesc.type == 0
        # assert cdesc.version == 0
        # assert cdesc.dim is None

        with pytest.raises(ValueError):
            cdesc = p.ifb.CFI_cdesc_t(-1)

        with pytest.raises(ValueError):
            cdesc = p.ifb.CFI_cdesc_t(p.ifb.CFI_MAX_RANK + 1)

        with pytest.raises(TypeError):
            cdesc = p.ifb.CFI_cdesc_t("a")

    def test_dim(self):
        z = p.ifb.CFI_cdesc_t(p.ifb.CFI_MAX_RANK)

        for i in range(0, p.ifb.CFI_MAX_RANK):
            assert z.dim[i].extent == 0

    def test_bytes(self):
        z = p.ifb.CFI_cdesc_t(5)

        by = z.to_bytes()

        assert isinstance(by, bytes)

        a = p.ifb.CFI_cdesc_t.from_bytes(by)

        assert a.rank == 5
