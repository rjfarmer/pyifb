# SPDX-License-Identifier: GPL-2.0+

import pyifb as p


class TestMacros:
    def test_macro_value(self):
        # Specific values we should know when they change
        assert p.CFI_VERSION == 1
        assert p.CFI_MAX_RANK == 15

    def test_macro_access(self):
        # Only want to make sure they exist
        assert p.CFI_attribute_pointer >= 0
        assert p.CFI_attribute_allocatable >= 0
        assert p.CFI_attribute_other >= 0