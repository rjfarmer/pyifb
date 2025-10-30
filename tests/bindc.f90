module bindc
    use iso_c_binding
    implicit none

    integer(C_INT), allocatable, bind(C,name='a_int_0d') :: a_int
    integer(C_INT), allocatable, bind(C,name='a_int_1d') :: a_int_1d(:)
    integer(C_INT), allocatable, bind(C,name='a_int_2d') :: a_int_2d(:,:)
    integer(C_INT), allocatable, bind(C,name='a_int_3d') :: a_int_3d(:,:,:)
    integer(C_INT), allocatable, bind(C,name='a_int_4d') :: a_int_4d(:,:,:,:)
    integer(C_INT), allocatable, bind(C,name='a_int_5d') :: a_int_5d(:,:,:,:,:)

    contains

    logical(c_bool) function is_alloc(x) bind(C, name='is_alloc')
        integer(C_INT), allocatable :: x(..)

        is_alloc = allocated(x)

    end function

end module bindc