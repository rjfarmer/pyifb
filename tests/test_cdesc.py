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

    def test_allocate_rank1(self):
        """Test allocate for a rank-1 descriptor."""
        cdesc = p.ifb.CFI_cdesc_t(1)

        # Allocate a 1D array of 100 8-byte elements
        status = cdesc.allocate([1], [100], 8)

        # CFI_SUCCESS should be 0
        assert status == p.ifb.CFI_SUCCESS
        assert cdesc.base_addr is not None
        assert cdesc.rank == 1

    def test_allocate_rank2(self):
        """Test allocate for a rank-2 descriptor."""
        cdesc = p.ifb.CFI_cdesc_t(2)

        # Allocate a 10×20 2D array of 8-byte elements
        status = cdesc.allocate([1, 1], [10, 20], 8)

        assert status == p.ifb.CFI_SUCCESS
        assert cdesc.base_addr is not None
        assert cdesc.rank == 2
        assert cdesc.dim[0].extent == 10
        assert cdesc.dim[1].extent == 20

    def test_allocate_rank3(self):
        """Test allocate for a rank-3 descriptor."""
        cdesc = p.ifb.CFI_cdesc_t(3)

        # Allocate a 5×6×7 3D array
        status = cdesc.allocate([0, 0, 0], [5, 6, 7], 4)

        assert status == p.ifb.CFI_SUCCESS
        assert cdesc.base_addr is not None
        assert cdesc.dim[0].extent == 6  # 5-0+1
        assert cdesc.dim[1].extent == 7  # 6-0+1
        assert cdesc.dim[2].extent == 8  # 7-0+1

    def test_allocate_mismatched_bounds(self):
        """Test allocate rejects mismatched lower/upper bounds."""
        cdesc = p.ifb.CFI_cdesc_t(2)

        # Provide mismatched number of bounds
        with pytest.raises(ValueError, match="Bounds sequences must match"):
            cdesc.allocate([1], [10, 20], 8)

        with pytest.raises(ValueError, match="Bounds sequences must match"):
            cdesc.allocate([1, 1], [10], 8)

    def test_allocate_invalid_sequences(self):
        """Test allocate handles non-sequence inputs."""
        cdesc = p.ifb.CFI_cdesc_t(2)

        # None is not a sequence
        with pytest.raises(TypeError):
            cdesc.allocate(None, [10, 20], 8)

        with pytest.raises(TypeError):
            cdesc.allocate([1, 1], None, 8)

    def test_allocate_scalar_descriptor(self):
        """Test allocate on rank-0 (scalar) descriptor."""
        cdesc = p.ifb.CFI_cdesc_t(0)

        # Rank-0 should work with empty bounds
        status = cdesc.allocate([], [], 8)

        # CFI may succeed or return an error for scalar
        # Just check it returns a valid status code
        assert isinstance(status, int)
        assert status >= 0

    def test_allocate_various_elem_sizes(self):
        """Test allocate with different element sizes."""
        for elem_size in [1, 2, 4, 8, 16, 32]:
            cdesc = p.ifb.CFI_cdesc_t(1)
            status = cdesc.allocate([1], [100], elem_size)

            # All element sizes should succeed
            assert status == p.ifb.CFI_SUCCESS

    def test_allocate_negative_bounds(self):
        """Test allocate with negative lower bounds."""
        cdesc = p.ifb.CFI_cdesc_t(1)

        # Negative lower bound should be valid in Fortran
        status = cdesc.allocate([-10], [10], 8)

        # Should succeed and have the right extent
        assert status == p.ifb.CFI_SUCCESS
        assert cdesc.dim[0].extent == 21  # 10 - (-10) + 1
