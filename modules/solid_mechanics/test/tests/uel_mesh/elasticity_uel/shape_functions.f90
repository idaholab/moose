MODULE SHAPE_FUNCTIONS
!C
CONTAINS
!C
    SUBROUTINE LINEAR_SHAPE_FUNCTION(N,G,X,Y)
!C
        USE CONSTANTS
        IMPLICIT NONE
!C
        REAL(R64), INTENT(IN) :: X
        REAL(R64), INTENT(IN) :: Y
!C
        REAL(R64), INTENT(OUT) :: N(1,4)
        REAL(R64), INTENT(OUT) :: G(2,4)
!C
        N(1,1) = + QUART*(ONE - X)*(ONE - Y)
        N(1,2) = + QUART*(ONE + X)*(ONE - Y)
        N(1,3) = + QUART*(ONE + X)*(ONE + Y)
        N(1,4) = + QUART*(ONE - X)*(ONE + Y)
!C
        G(1,1) = - QUART*(ONE - Y)
        G(1,2) = + QUART*(ONE - Y)
        G(1,3) = + QUART*(ONE + Y)
        G(1,4) = - QUART*(ONE + Y)
!C
        G(2,1) = - QUART*(ONE - X)
        G(2,2) = - QUART*(ONE + X)
        G(2,3) = + QUART*(ONE + X)
        G(2,4) = + QUART*(ONE - X)
!C
    END SUBROUTINE LINEAR_SHAPE_FUNCTION
!C
    SUBROUTINE QUAD_SHAPE_FUNCTION(N,G,X,Y)
!C
        USE CONSTANTS
        IMPLICIT NONE
!C
        REAL(R64), INTENT(IN) :: X
        REAL(R64), INTENT(IN) :: Y
!C
        REAL(R64), INTENT(OUT) :: N(1,8)
        REAL(R64), INTENT(OUT) :: G(2,8)
!C
        N(1,1) = - QUART*(ONE - X)*(ONE - Y)*(ONE + X + Y)
        N(1,2) = - QUART*(ONE + X)*(ONE - Y)*(ONE - X + Y)
        N(1,3) = - QUART*(ONE + X)*(ONE + Y)*(ONE - X - Y)
        N(1,4) = - QUART*(ONE - X)*(ONE + Y)*(ONE + X - Y)
        N(1,5) = + HALF*(ONE - X*X)*(ONE - Y)
        N(1,6) = + HALF*(ONE + X)*(ONE - Y*Y)
        N(1,7) = + HALF*(ONE - X*X)*(ONE + Y)
        N(1,8) = + HALF*(ONE - X)*(ONE - Y*Y)
!C
        G(1,1) = + QUART*(ONE - Y)*(TWO*X + Y)
        G(1,2) = + QUART*(ONE - Y)*(TWO*X - Y)
        G(1,3) = + QUART*(ONE + Y)*(TWO*X + Y)
        G(1,4) = + QUART*(ONE + Y)*(TWO*X - Y)
        G(1,5) = - X*(ONE - Y)
        G(1,6) = + HALF*(ONE - Y*Y)
        G(1,7) = - X*(ONE + Y)
        G(1,8) = - HALF*(ONE - Y*Y)
!C
        G(2,1) = + QUART*(ONE - X)*(X + 2*Y)
        G(2,2) = - QUART*(ONE + X)*(X - 2*Y)
        G(2,3) = + QUART*(ONE + X)*(X + 2*Y)
        G(2,4) = - QUART*(ONE - X)*(X - 2*Y)
        G(2,5) = - HALF*(ONE - X*X)
        G(2,6) = - Y*(ONE + X)
        G(2,7) = + HALF*(ONE - X*X)
        G(2,8) = - Y*(ONE - X)
!C
    END SUBROUTINE QUAD_SHAPE_FUNCTION
!C
    SUBROUTINE FIELD_MAP_FUNCTION(N,X,Y)
        !C
                USE CONSTANTS
                IMPLICIT NONE
        !C
                REAL(R64), INTENT(IN) :: X
                REAL(R64), INTENT(IN) :: Y
        !C
                REAL(R64), INTENT(OUT) :: N(1,9)
        !C
                N(1,1) = + QUART*(ONE - X)*(ONE - Y)
                N(1,2) = + QUART*(ONE + X)*(ONE - Y)
                N(1,3) = + QUART*(ONE + X)*(ONE + Y)
                N(1,4) = + QUART*(ONE - X)*(ONE + Y)
                N(1,5) = ZERO
                N(1,6) = ZERO
                N(1,7) = ZERO
                N(1,8) = ZERO
                N(1,9) = ZERO
        !C
    END SUBROUTINE FIELD_MAP_FUNCTION
!C
END MODULE
