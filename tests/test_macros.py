# SPDX-License-Identifier: GPL-2.0+

import pyifb as p

import pytest


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

    def test_failures(self):
        # Test return codes

        # Only one that has a defined value
        assert p.CFI_SUCCESS == 0

        # A value other than 0
        assert p.CFI_ERROR_BASE_ADDR_NULL != 0
        assert p.CFI_ERROR_BASE_ADDR_NOT_NULL != 0
        assert p.CFI_INVALID_RANK != 0
        assert p.CFI_INVALID_TYPE != 0
        assert p.CFI_INVALID_ATTRIBUTE != 0
        assert p.CFI_INVALID_DESCRIPTOR != 0
        assert p.CFI_ERROR_MEM_ALLOCATION != 0 
        assert p.CFI_ERROR_OUT_OF_BOUNDS != 0

    def test_compiler(self):
        assert p.IFB_COMPILER != "UNKNOWN"
        
        if p.IFB_COMPILER == 'GCC':
            assert len(p.IFB_COMPILER_VERSION)

    def test_types(self):
        assert p.CFI_type_signed_char > 0
        assert p.CFI_type_short > 0
        assert p.CFI_type_int > 0
        assert p.CFI_type_long > 0
        assert p.CFI_type_long_long > 0
        assert p.CFI_type_size_t > 0
        assert p.CFI_type_int8_t > 0
        assert p.CFI_type_int16_t > 0
        assert p.CFI_type_int32_t > 0
        assert p.CFI_type_int64_t > 0
        assert p.CFI_type_int_least8_t > 0
        assert p.CFI_type_int_least16_t > 0
        assert p.CFI_type_int_least32_t > 0
        assert p.CFI_type_int_least64_t > 0
        assert p.CFI_type_int_fast8_t > 0
        assert p.CFI_type_int_fast16_t > 0
        assert p.CFI_type_int_fast32_t > 0
        assert p.CFI_type_int_fast64_t > 0
        assert p.CFI_type_intmax_t > 0
        assert p.CFI_type_intptr_t > 0
        assert p.CFI_type_ptrdiff_t > 0
        assert p.CFI_type_float > 0
        assert p.CFI_type_double > 0
        assert p.CFI_type_long_double > 0
        assert p.CFI_type_float_Complex > 0
        assert p.CFI_type_double_Complex > 0
        assert p.CFI_type_long_double_Complex > 0
        assert p.CFI_type_Bool > 0
        assert p.CFI_type_char > 0
        assert p.CFI_type_cptr > 0
        assert p.CFI_type_struct > 0

    @pytest.mark.skipif(p.IFB_COMPILER != 'GCC',reason='Only for GCC')
    def test_gcc_types(self):
        assert p.CFI_type_mask == 0xFF
        assert p.CFI_type_kind_shift == 8
        assert p.CFI_type_Integer == 1
        assert p.CFI_type_Logical == 2
        assert p.CFI_type_Real == 3
        assert p.CFI_type_Complex == 4
        assert p.CFI_type_Character == 5
        assert p.CFI_type_struct == 6
        assert p.CFI_type_cptr == 7
        assert p.CFI_type_cfunptr == 8
        assert p.CFI_type_other == -1

    def test_cfi_dim(self):
        x = p.CFI_dim_t()
        assert x.sm == 0
        assert x.lower_bound == 0
        assert x.extent == 0

        with pytest.raises(TypeError) as e:
            x.extent = 'a'

        with pytest.raises(TypeError) as e:
            x.sm = 'a'

        with pytest.raises(TypeError) as e:
            x.lower_bound = 'a'

        x.lower_bound  = 1
        x.sm = 1
        x.extent = 1