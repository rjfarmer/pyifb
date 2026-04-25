from typing import Final, TypeVar

_CFI_cdesc_T = TypeVar("_CFI_cdesc_T", bound="CFI_cdesc_t")

class CFI_dim_t:
    lower_bound: int
    extent: int
    sm: int

class CFI_cdesc_t:
    def __new__(cls, _rank: int = 0) -> "CFI_cdesc_t": ...
    @classmethod
    def from_bytes(cls: type[_CFI_cdesc_T], _arg: bytes) -> _CFI_cdesc_T: ...
    @classmethod
    def from_param(cls, _arg: "CFI_cdesc_t") -> bytes: ...
    def to_bytes(self) -> bytes: ...
    def allocate(
        self,
        _lower_bounds: list[int] | tuple[int, ...],
        _upper_bounds: list[int] | tuple[int, ...],
        _elem_len: int,
        _type_code: int = 0,
    ) -> int: ...
    def deallocate(self) -> int: ...
    def establish(
        self,
        _base_addr: int | None,
        _attribute: int,
        _type_code: int,
        _elem_len: int,
        _rank: int,
        _extents: list[int] | tuple[int, ...],
    ) -> int: ...
    def is_contiguous(self) -> bool: ...
    def section(
        self,
        _result_desc: "CFI_cdesc_t",
        _lower_bounds: list[int] | tuple[int, ...] | None = None,
        _upper_bounds: list[int] | tuple[int, ...] | None = None,
        _strides: list[int] | tuple[int, ...] | None = None,
    ) -> int: ...
    def select_part(
        self, _result_desc: "CFI_cdesc_t", _displacement: int, _elem_len: int
    ) -> int: ...
    def setpointer(
        self,
        _source_desc: "CFI_cdesc_t" | None,
        _lower_bounds: list[int] | tuple[int, ...] | None = None,
    ) -> int: ...
    @property
    def base_addr(self) -> int | None: ...
    @property
    def rank(self) -> int: ...
    @property
    def attribute(self) -> int: ...
    @property
    def elem_len(self) -> int: ...
    @property
    def type(self) -> int: ...
    @property
    def version(self) -> int: ...
    @property
    def dim(self) -> tuple[CFI_dim_t, ...] | None: ...
    @property
    def _as_parameter(self) -> bytes: ...

def _offsetof_cdesc() -> dict[str, int]: ...

CFI_MAX_RANK: Final[int]
CFI_VERSION: Final[int]

CFI_attribute_pointer: Final[int]
CFI_attribute_allocatable: Final[int]
CFI_attribute_other: Final[int]

CFI_SUCCESS: Final[int]
CFI_ERROR_BASE_ADDR_NULL: Final[int]
CFI_ERROR_BASE_ADDR_NOT_NULL: Final[int]
CFI_INVALID_RANK: Final[int]
CFI_INVALID_TYPE: Final[int]
CFI_INVALID_ATTRIBUTE: Final[int]
CFI_INVALID_EXTENT: Final[int]
CFI_INVALID_DESCRIPTOR: Final[int]
CFI_ERROR_MEM_ALLOCATION: Final[int]
CFI_ERROR_OUT_OF_BOUNDS: Final[int]
CFI_FAILURE: Final[int]
CFI_INVALID_STRIDE: Final[int]

_sizeof_cdesc: Final[int]
_sizeof_dims: Final[int]
_sizeof_cfi_rank_t: Final[int]
_sizeof_cfi_attribute_t: Final[int]
_sizeof_cfi_type_t: Final[int]

IFB_COMPILER: Final[str]
IFB_COMPILER_VERSION: Final[str]

CFI_type_signed_char: Final[int]
CFI_type_short: Final[int]
CFI_type_int: Final[int]
CFI_type_long: Final[int]
CFI_type_long_long: Final[int]
CFI_type_size_t: Final[int]
CFI_type_int8_t: Final[int]
CFI_type_int16_t: Final[int]
CFI_type_int32_t: Final[int]
CFI_type_int64_t: Final[int]
CFI_type_int_least8_t: Final[int]
CFI_type_int_least16_t: Final[int]
CFI_type_int_least32_t: Final[int]
CFI_type_int_least64_t: Final[int]
CFI_type_int_fast8_t: Final[int]
CFI_type_int_fast16_t: Final[int]
CFI_type_int_fast32_t: Final[int]
CFI_type_int_fast64_t: Final[int]
CFI_type_intmax_t: Final[int]
CFI_type_intptr_t: Final[int]
CFI_type_ptrdiff_t: Final[int]
CFI_type_float: Final[int]
CFI_type_double: Final[int]
CFI_type_long_double: Final[int]
CFI_type_float_Complex: Final[int]
CFI_type_double_Complex: Final[int]
CFI_type_long_double_Complex: Final[int]
CFI_type_Bool: Final[int]
CFI_type_char: Final[int]
CFI_type_cptr: Final[int]
CFI_type_struct: Final[int]
CFI_type_other: Final[int]

CFI_type_mask: Final[int]
CFI_type_kind_shift: Final[int]
CFI_type_Integer: Final[int]
CFI_type_Logical: Final[int]
CFI_type_Real: Final[int]
CFI_type_Complex: Final[int]
CFI_type_Character: Final[int]
CFI_type_cfunptr: Final[int]
CFI_type_float128: Final[int]
CFI_type_float128_Complex: Final[int]
