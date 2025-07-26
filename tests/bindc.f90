module bindc
    use iso_c_binding
    implicit none

    integer(C_INT), bind(C,name='a_int_0d') :: a_int
    integer(C_INT), bind(C,name='a_int_1d') :: a_int_1d(1)
    integer(C_INT), bind(C,name='a_int_2d') :: a_int_2d(1,2)
    integer(C_INT), bind(C,name='a_int_3d') :: a_int_3d(1,2,3)
    integer(C_INT), bind(C,name='a_int_4d') :: a_int_4d(1,2,3,4)
    integer(C_INT), bind(C,name='a_int_5d') :: a_int_5d(1,2,3,4,5)

    contains



end module bindc