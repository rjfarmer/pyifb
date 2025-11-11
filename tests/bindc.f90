module bindc
    use iso_c_binding
    implicit none

    contains

    logical(c_bool) function is_alloc(x) bind(C, name='is_alloc')
        integer(C_INT), allocatable, intent(in)  :: x(..)

        is_alloc = allocated(x)

    end function

end module bindc