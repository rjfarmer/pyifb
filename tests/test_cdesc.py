# SPDX-License-Identifier: GPL-2.0+

import pytest

import pyifb as p


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

    def test_dim_isinstance(self):
        """dim elements must be instances of the exported CFI_dim_t type."""
        z = p.ifb.CFI_cdesc_t(2)
        z.establish(0, p.ifb.CFI_attribute_other, p.ifb.CFI_type_int, 4, 2, [3, 4])
        dims = z.dim
        assert isinstance(dims[0], p.ifb.CFI_dim_t)
        assert isinstance(dims[1], p.ifb.CFI_dim_t)

    def test_dim_type_identity(self):
        """All dim elements share exactly the same type object."""
        z = p.ifb.CFI_cdesc_t(3)
        z.establish(0, p.ifb.CFI_attribute_other, p.ifb.CFI_type_int, 4, 3, [2, 3, 4])
        dims = z.dim
        assert type(dims[0]) is p.ifb.CFI_dim_t
        assert type(dims[0]) is type(dims[1])
        assert type(dims[1]) is type(dims[2])

    def test_bytes(self):
        z = p.ifb.CFI_cdesc_t(5)

        by = z.to_bytes()

        assert isinstance(by, bytes)

        a = p.ifb.CFI_cdesc_t.from_bytes(by)

        assert a.rank == 5

    def test_offsets(self):
        offsets = p.ifb._offsetof_cdesc()

        assert offsets["base_addr"] == 0
        assert offsets["base_addr"] < offsets["elem_len"]
        assert offsets["elem_len"] < offsets["version"]
        assert offsets["version"] < offsets["dim"]
        assert offsets["attribute"] < offsets["dim"]
        assert offsets["rank"] < offsets["dim"]
        assert offsets["type"] < offsets["dim"]
        assert offsets["dim"] <= p.ifb._sizeof_cdesc

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
        with pytest.raises(
            ValueError, match="Bounds sequences must have at least rank elements"
        ):
            cdesc.allocate([1], [10, 20], 8)

        with pytest.raises(
            ValueError, match="Bounds sequences must have at least rank elements"
        ):
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

        assert isinstance(status, int)
        assert status == p.ifb.CFI_SUCCESS

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

    def test_deallocate(self):
        """Test deallocate function."""
        cdesc = p.ifb.CFI_cdesc_t(1)

        # Allocate first
        status = cdesc.allocate([1], [100], 8)
        assert status == p.ifb.CFI_SUCCESS
        assert cdesc.base_addr is not None

        # Deallocate
        status = cdesc.deallocate()
        assert status == p.ifb.CFI_SUCCESS
        # Note: base_addr may still be set after deallocate, depending on CFI implementation

    def test_establish(self):
        """Test establish function."""
        import ctypes

        cdesc = p.ifb.CFI_cdesc_t(2)

        # Test establish with NULL base_addr using ctypes
        status = cdesc.establish(
            None,
            p.ifb.CFI_attribute_other,
            p.ifb.CFI_type_int,
            4,
            2,
            [2, 3],
        )

        assert status == p.ifb.CFI_SUCCESS
        assert cdesc.rank == 2
        # Extent is ignored when base address is NULL
        assert cdesc.dim[0].extent == 0

    def test_is_contiguous(self):
        """Test is_contiguous function."""
        cdesc = p.ifb.CFI_cdesc_t(2)

        # Allocate a contiguous array
        status = cdesc.allocate([1, 1], [10, 20], 8)
        assert status == p.ifb.CFI_SUCCESS

        # Should be contiguous
        assert cdesc.is_contiguous()

    def test_section(self):
        """Test section function."""
        cdesc = p.ifb.CFI_cdesc_t(2)

        # Allocate a 10x20 array
        status = cdesc.allocate([1, 1], [10, 20], 8)
        assert status == p.ifb.CFI_SUCCESS

        # Create a section (subarray)
        section = p.ifb.CFI_cdesc_t(2)
        status = cdesc.section(
            section, [2, 5], [5, 10], [1, 1]
        )  # result, lower_bounds, upper_bounds, strides

        assert status == p.ifb.CFI_SUCCESS
        assert section.rank == 2
        assert section.dim[0].extent == 4  # 5-2+1
        assert section.dim[1].extent == 6  # 10-5+1

    def test_select_part(self):
        """Test select_part function."""
        cdesc = p.ifb.CFI_cdesc_t(1)

        # Allocate an array of structs (each struct is 16 bytes)
        status = cdesc.allocate([1], [100], 16)
        assert status == p.ifb.CFI_SUCCESS

        # Select a part (e.g., second field at offset 8, size 4)
        part = p.ifb.CFI_cdesc_t(1)  # Same rank as source
        # Establish the result descriptor with appropriate properties
        status = part.establish(
            None, p.ifb.CFI_attribute_other, p.ifb.CFI_type_int, 4, 1, [100]
        )
        assert status == p.ifb.CFI_SUCCESS

        status = cdesc.select_part(part, 8, 4)

        assert status == p.ifb.CFI_SUCCESS
        # The result should have the same rank as the source for array elements
        assert part.rank == 1
        assert part.dim[0].extent == 100  # Same number of elements

    def test_setpointer(self):
        """Test setpointer function."""
        cdesc1 = p.ifb.CFI_cdesc_t(2)
        cdesc2 = p.ifb.CFI_cdesc_t(2)

        # Allocate first descriptor
        status = cdesc1.allocate([1, 1], [10, 20], 8)
        assert status == p.ifb.CFI_SUCCESS

        # Set second descriptor to point to first with explicit lower bounds
        status = cdesc2.setpointer(cdesc1, [1, 1])

        assert status == p.ifb.CFI_SUCCESS
        # If it succeeds, check some properties
        if status == p.ifb.CFI_SUCCESS:
            assert cdesc2.rank == cdesc1.rank
            assert cdesc2.base_addr == cdesc1.base_addr

    def test_establish_rejects_when_descriptor_owns_memory(self):
        cdesc = p.ifb.CFI_cdesc_t(1)
        status = cdesc.allocate([1], [10], 4)
        assert status == p.ifb.CFI_SUCCESS

        with pytest.raises(ValueError, match="must be deallocated before establish"):
            cdesc.establish(
                None,
                p.ifb.CFI_attribute_other,
                p.ifb.CFI_type_int,
                4,
                1,
                [10],
            )

    def test_section_rejects_when_result_descriptor_owns_memory(self):
        src = p.ifb.CFI_cdesc_t(2)
        dst = p.ifb.CFI_cdesc_t(2)

        assert src.allocate([1, 1], [10, 20], 8) == p.ifb.CFI_SUCCESS
        assert dst.allocate([1, 1], [10, 20], 8) == p.ifb.CFI_SUCCESS

        with pytest.raises(ValueError, match="must be deallocated before section"):
            src.section(dst, [2, 5], [5, 10], [1, 1])

    def test_select_part_rejects_when_result_descriptor_owns_memory(self):
        src = p.ifb.CFI_cdesc_t(1)
        dst = p.ifb.CFI_cdesc_t(1)

        assert src.allocate([1], [100], 16) == p.ifb.CFI_SUCCESS
        assert dst.allocate([1], [100], 4) == p.ifb.CFI_SUCCESS

        with pytest.raises(ValueError, match="must be deallocated before select_part"):
            src.select_part(dst, 8, 4)

    def test_setpointer_rejects_when_descriptor_owns_memory(self):
        dst = p.ifb.CFI_cdesc_t(1)
        src = p.ifb.CFI_cdesc_t(1)

        assert dst.allocate([1], [5], 4) == p.ifb.CFI_SUCCESS
        assert src.allocate([1], [5], 4) == p.ifb.CFI_SUCCESS

        with pytest.raises(ValueError, match="must be deallocated before setpointer"):
            dst.setpointer(src, [1])
