# SPDX-License-Identifier: GPL-2.0+

import pyifb as p


class TestMacros:
    def test_macro(self):
        assert p.CFI_VERSION == 1
        assert p.CFI_MAX_RANK == 15