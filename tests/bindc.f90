module bindc
    use iso_c_binding
    implicit none

    contains

    logical(c_bool) function is_alloc(x) bind(C, name='is_alloc')
        integer(C_INT), allocatable, intent(in)  :: x(..)

        is_alloc = allocated(x)

    end function

    integer(c_int) function add_ints(a, b) bind(C, name='add_ints')
        integer(c_int), value, intent(in) :: a, b

        add_ints = a + b

    end function

    real(c_double) function scale_double(x, factor) bind(C, name='scale_double')
        real(c_double), value, intent(in) :: x, factor

        scale_double = x * factor

    end function

    integer(c_int) function array_sum(x, n) bind(C, name='array_sum')
        integer(c_int), value, intent(in) :: n
        integer(c_int), intent(in) :: x(n)
        integer(c_int) :: i

        array_sum = 0
        do i = 1, n
            array_sum = array_sum + x(i)
        end do

    end function

    subroutine fill_array(x, n, val) bind(C, name='fill_array')
        integer(c_int), value, intent(in) :: n, val
        integer(c_int), intent(out) :: x(n)

        x = val

    end subroutine

    subroutine shift_chars(x, n, offset) bind(C, name='shift_chars')
        integer(c_int), value, intent(in) :: n, offset
        character(kind=c_char), intent(inout) :: x(n)
        integer(c_int) :: i

        do i = 1, n
            x(i) = achar(iachar(x(i)) + offset, kind=c_char)
        end do

    end subroutine

end module bindc