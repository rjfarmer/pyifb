module bindc
    use iso_c_binding
    implicit none

    type, bind(C) :: pair_t
        integer(c_int) :: x
        real(c_double) :: y
    end type pair_t

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

    integer(c_int) function pair_sum(v) bind(C, name='pair_sum')
        type(pair_t), intent(in) :: v

        pair_sum = v%x + int(v%y, kind=c_int)

    end function

    integer(c_int) function pair_array_sum_explicit(v, n) bind(C, name='pair_array_sum_explicit')
        integer(c_int), value, intent(in) :: n
        type(pair_t), intent(in) :: v(n)
        integer(c_int) :: i

        pair_array_sum_explicit = 0
        do i = 1, n
            pair_array_sum_explicit = pair_array_sum_explicit + v(i)%x + int(v(i)%y, kind=c_int)
        end do

    end function

    integer(c_int) function pair_array_sum_alloc(v) bind(C, name='pair_array_sum_alloc')
        type(pair_t), allocatable, intent(in) :: v(:)
        integer(c_int) :: i

        if (.not. allocated(v)) then
            pair_array_sum_alloc = -1
            return
        end if

        pair_array_sum_alloc = 0
        do i = lbound(v, 1), ubound(v, 1)
            pair_array_sum_alloc = pair_array_sum_alloc + v(i)%x + int(v(i)%y, kind=c_int)
        end do

    end function

end module bindc