MODULE GAUSS_QUADRATURE
    CONTAINS
!C
        SUBROUTINE GAUSS_WEIGHTS_LOCS(WEIGHTS,POINTS,N)
!C
            USE CONSTANTS
            IMPLICIT NONE
!C
            INTEGER, INTENT(IN) :: N
!C
            REAL(R64), INTENT(OUT) :: WEIGHTS(N)
            REAL(R64), INTENT(OUT) :: POINTS(N)
!C
            SELECT CASE (N)
            CASE (1)
!C
                WEIGHTS(1) = 2.0_R64
!C
                POINTS(1) = 0.0_R64
!C
            CASE (2)
!C
                WEIGHTS(1) = 1.0_R64
                WEIGHTS(2) = WEIGHTS(1)
!C
                POINTS(1) = -5.7735026918962576450914878050196E-01_R64
                POINTS(2) = -POINTS(1)
!C
            CASE(3)
!C
                WEIGHTS(1) = 5.5555555555555555555555555555556E-01_R64
                WEIGHTS(2) = 8.8888888888888888888888888888889E-01_R64
                WEIGHTS(3) = WEIGHTS(1)
!C
                POINTS(1) = -7.7459666924148337703585307995648E-01_R64
                POINTS(2) = 0.0_R64
                POINTS(3) = -POINTS(1)
!C
            CASE DEFAULT
!C
                return
!C
            END SELECT
!C
        END SUBROUTINE GAUSS_WEIGHTS_LOCS
END MODULE GAUSS_QUADRATURE
