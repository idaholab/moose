    SUBROUTINE PHYSICS_EQUATIONS(RES, JAC,                             &
    &                       U1, U2, U3, P,                             &
    &                       DU1, DU2, DU3, DP,                         &
    &                       U1_OLD, U2_OLD, U3_OLD, P_OLD,             &
    &                       GRAD_U1, GRAD_U2,                          &
    &                       GRAD_DU1, GRAD_DU2,                        &
    &                       GRAD_U1_OLD, GRAD_U2_OLD,                  &
    &                       GRAD_P, GRAD_U3, GRAD_DU3, GRAD_U3_OLD,    &
    &                       PROPS, NPROPS,                             &
    &                       LOCAL_PRED, LOCAL_DPRED, NPREDF,           &
    &                       N1, N2, N3, N4,                            &
    &                       NT1, NT2, NT3, NT4,                        &
    &                       B1, B2, B3, B4,                            &
    &                       BT1, BT2, BT3, BT4,                        &
    &                       DTIME, TIME, NDOFEL, MCRD,                 &
    &                       LOCAL_STATV, NSTATV)
!C
    USE CONSTANTS
    USE MATRIX_OPERATIONS
!C
    IMPLICIT NONE
!C
    REAL(R64), INTENT(OUT) :: RES(NDOFEL), JAC(NDOFEL,NDOFEL)
!C
    REAL(R64), INTENT(OUT) :: LOCAL_STATV(NSTATV)
!C
    REAL(R64), INTENT(IN) :: U1, U2, U3, P, DTIME, PROPS(NPROPS)
    REAL(R64), INTENT(IN) :: DU1, DU2, DU3, DP
    REAL(R64), INTENT(IN) :: U1_OLD, U2_OLD, P_OLD, U3_OLD
    REAL(R64), INTENT(IN) :: GRAD_U1(MCRD), GRAD_U2(MCRD), TIME(2)
    REAL(R64), INTENT(IN) :: GRAD_DU1(MCRD), GRAD_DU2(MCRD)
    REAL(R64), INTENT(IN) :: GRAD_U1_OLD(MCRD), GRAD_U2_OLD(MCRD)
    REAL(R64), INTENT(IN) :: GRAD_P(MCRD)
    REAL(R64), INTENT(IN) :: GRAD_U3, GRAD_DU3, GRAD_U3_OLD
    REAL(R64), INTENT(IN) :: LOCAL_PRED(NPREDF), LOCAL_DPRED(NPREDF)
    REAL(R64), INTENT(IN) :: N1(1,NDOFEL), NT1(NDOFEL,1)
    REAL(R64), INTENT(IN) :: N2(1,NDOFEL), NT2(NDOFEL,1)
    REAL(R64), INTENT(IN) :: N3(1,NDOFEL), NT3(NDOFEL,1)
    REAL(R64), INTENT(IN) :: N4(1,NDOFEL), NT4(NDOFEL,1)
    REAL(R64), INTENT(IN) :: B1(MCRD,NDOFEL), BT1(NDOFEL,MCRD)
    REAL(R64), INTENT(IN) :: B2(MCRD,NDOFEL), BT2(NDOFEL,MCRD)
    REAL(R64), INTENT(IN) :: B3(MCRD,NDOFEL), BT3(NDOFEL,MCRD)
    REAL(R64), INTENT(IN) :: B4(1,NDOFEL), BT4(NDOFEL,1)
!C
    INTEGER, INTENT(IN) :: NPROPS, NPREDF, NDOFEL, MCRD, NSTATV
!C
!C  Local Variables
!C
    INTEGER, PARAMETER :: MNPROPS = 7
    INTEGER, PARAMETER :: MNSTATV = 1
    INTEGER, PARAMETER :: NTENS = 4
    INTEGER, PARAMETER :: NDI = 3
    INTEGER, PARAMETER :: NSHR = 1
!C
    INTEGER :: I, J
!C
    REAL(R64) :: INV_DTIME, RHO
    REAL(R64) :: MPROPS(MNPROPS)
    REAL(R64) :: CAUCHY(3,3), IDENTITY(3,3), I_VEC(NTENS,1)
    REAL(R64) :: CAUCHY_BAR(3,3), HYDRO
    REAL(R64) :: DSTRAN_DU(NTENS,NDOFEL)
    REAL(R64) :: DCAUCHY_DU(NTENS,NDOFEL), DHYDRO_DU(1,NDOFEL)
    REAL(R64) :: DCAUCHY_BAR_DU(NTENS,NDOFEL), DCAUCHY_BAR_DP(NTENS,NDOFEL)
    REAL(R64) :: DEV(3,3), D_HYDRO, HYDRO_OLD, BULK_K, INV_BULK_K
!C
!C  UMAT Variables
!C
    REAL(R64) :: TEMP, DTEMP, MSTATV(MNSTATV), DDSDDE(NTENS,NTENS)
    REAL(R64) :: DSTRAN(NTENS), STRAN(NTENS), PREDEF(1), DPRED(1), STRESS(NTENS)
    REAL(R64) :: SSE, SPD, SCD, DDSDDT(NTENS), RPL, DRPLDT, PNEWDT
    REAL(R64) :: COORDS(3), DROT(3,3), DRPLDE(NTENS)
    REAL(R64) :: CELENT, DFGRD0(3,3), DFGRD1(3,3)
!C
    INTEGER :: NOEL, NPT, LAYER, KSPT, KSTEP, KINC
!C
    CHARACTER :: CMNAME(80)
!C
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C  INITIALIZE AND UNPACK
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C
    RES = ZERO
    JAC = ZERO
!C
    INV_DTIME = ZERO
    IF (DTIME .GT. ZERO) INV_DTIME = ONE/DTIME
!C
    TEMP = LOCAL_PRED(1)
    DTEMP = LOCAL_DPRED(1)
    PREDEF(1) = LOCAL_PRED(2)
    DPRED(1) = LOCAL_DPRED(2)
!C
    MPROPS(1) = PROPS(1)!E0
    MPROPS(2) = PROPS(2)!E1*T
    MPROPS(3) = PROPS(3)!NU0
    MPROPS(4) = PROPS(4)!NU1*T
    MPROPS(5) = PROPS(5)!ALPHA0
    MPROPS(6) = PROPS(6)!ALPHA1*T
    MPROPS(7) = PROPS(7)!PPM_SCALAR
!C
    RHO = PROPS(8)
!C
    MSTATV(1) = LOCAL_STATV(1)!SIG_EQ
    SSE = LOCAL_STATV(2)!ELASTIC_ENERGY
!C
    STRESS(1) = LOCAL_STATV(3)
    STRESS(2) = LOCAL_STATV(4)
    STRESS(3) = LOCAL_STATV(5)
    STRESS(4) = LOCAL_STATV(6)
!C
    STRAN(1) = LOCAL_STATV(7)
    STRAN(2) = LOCAL_STATV(8)
    STRAN(3) = LOCAL_STATV(9)
    STRAN(4) = LOCAL_STATV(10)
!C
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C  BEGIN STRUCTURAL EQUATIONS
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C
    I_VEC = ONE
    I_VEC(4,1) = ZERO
!C
    IDENTITY = ZERO
    DO I = 1, 3
        IDENTITY(I,I) = ONE
    ENDDO
!C
    CAUCHY = ZERO
    CAUCHY(1,1) = STRESS(1)
    CAUCHY(2,2) = STRESS(2)
    CAUCHY(3,3) = STRESS(3)
    CAUCHY(1,2) = STRESS(4)
    CAUCHY(2,1) = STRESS(4)
!C
    HYDRO_OLD = THIRD*(CAUCHY(1,1) + CAUCHY(2,2) + CAUCHY(3,3))
!C
    DSTRAN(1) = GRAD_DU1(1)
    DSTRAN(2) = GRAD_DU2(2)
    DSTRAN(3) = GRAD_DU3
    DSTRAN(4) = GRAD_DU1(2) + GRAD_DU2(1)
!C
    CALL UMAT(STRESS, MSTATV, DDSDDE, SSE, SPD, SCD, RPL, DDSDDT,      &
    &         DRPLDE, DRPLDT, STRAN, DSTRAN, TIME, DTIME, TEMP, DTEMP, &
    &         PREDEF, DPRED, CMNAME, NDI, NSHR, NTENS, MNSTATV, MPROPS,&
    &         MNPROPS, COORDS, DROT, PNEWDT, CELENT, DFGRD0, DFGRD1,   &
    &         NOEL, NPT, LAYER, KSPT, KSTEP, KINC)
!C
    BULK_K = (1.0_R64)/(9.0_R64)*sum(DDSDDE(1:3,1:3))
    INV_BULK_K = ONE/BULK_K
!C
    CAUCHY = ZERO
    CAUCHY(1,1) = STRESS(1)
    CAUCHY(2,2) = STRESS(2)
    CAUCHY(3,3) = STRESS(3)
    CAUCHY(1,2) = STRESS(4)
    CAUCHY(2,1) = STRESS(4)
!C
    HYDRO = THIRD*(CAUCHY(1,1) + CAUCHY(2,2) + CAUCHY(3,3))
    D_HYDRO = HYDRO - HYDRO_OLD
!C
    DEV = CAUCHY - IDENTITY*HYDRO
    CAUCHY_BAR = CAUCHY - (ONE - RHO)*IDENTITY*(D_HYDRO + DP)
!C
    RES = RES + matmul(BT1,CAUCHY_BAR(1:2,1))
    RES = RES + matmul(BT2,CAUCHY_BAR(1:2,2))
    RES = RES + BT4(:,1)*CAUCHY_BAR(3,3)
!C
    RES = RES + NT3(:,1)*INV_BULK_K*(D_HYDRO + DP)
!C
!C  Jacobian Contributions
!C
    DSTRAN_DU(1,:) = B1(1,:)
    DSTRAN_DU(2,:) = B2(2,:)
    DSTRAN_DU(3,:) = B4(1,:)
    DSTRAN_DU(4,:) = B1(2,:) + B2(1,:)
!C
    DCAUCHY_DU = matmul(DDSDDE,DSTRAN_DU)
!C
    DHYDRO_DU(1,:) = THIRD*(DCAUCHY_DU(1,:)+DCAUCHY_DU(2,:)+DCAUCHY_DU(3,:))
!C
    DCAUCHY_BAR_DU = DCAUCHY_DU - (ONE - RHO)*matmul(I_VEC,DHYDRO_DU)
!C
    DCAUCHY_BAR_DP = - (ONE - RHO)*matmul(I_VEC,N3)
!C
    DO I = 1, NDOFEL
        DO J = 1, NDOFEL
            JAC(I,J) = JAC(I,J) + BT1(I,1)*DCAUCHY_BAR_DU(1,J)
            JAC(I,J) = JAC(I,J) + BT1(I,2)*DCAUCHY_BAR_DU(4,J)
            JAC(I,J) = JAC(I,J) + BT2(I,1)*DCAUCHY_BAR_DU(4,J)
            JAC(I,J) = JAC(I,J) + BT2(I,2)*DCAUCHY_BAR_DU(2,J)
            JAC(I,J) = JAC(I,J) + BT4(I,1)*DCAUCHY_BAR_DU(3,J)
!C
            JAC(I,J) = JAC(I,J) + BT1(I,1)*DCAUCHY_BAR_DP(1,J)
            JAC(I,J) = JAC(I,J) + BT1(I,2)*DCAUCHY_BAR_DP(4,J)
            JAC(I,J) = JAC(I,J) + BT2(I,1)*DCAUCHY_BAR_DP(4,J)
            JAC(I,J) = JAC(I,J) + BT2(I,2)*DCAUCHY_BAR_DP(2,J)
            JAC(I,J) = JAC(I,J) + BT4(I,1)*DCAUCHY_BAR_DP(3,J)
        END DO
    END DO
!C
    JAC = JAC + INV_BULK_K*matmul(NT3,DHYDRO_DU)
    JAC = JAC + INV_BULK_K*matmul(NT3,N3)
!C
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C  END STRUCTURAL EQUATIONS
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C
!C
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C  BEGIN PACK
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C
    LOCAL_STATV(1) = MSTATV(1)
    LOCAL_STATV(2) = SSE
!C
    LOCAL_STATV(3) = STRESS(1)
    LOCAL_STATV(4) = STRESS(2)
    LOCAL_STATV(5) = STRESS(3)
    LOCAL_STATV(6) = STRESS(4)
!C
    LOCAL_STATV( 7) = STRAN(1) + DSTRAN(1)
    LOCAL_STATV( 8) = STRAN(2) + DSTRAN(2)
    LOCAL_STATV( 9) = STRAN(3) + DSTRAN(3)
    LOCAL_STATV(10) = STRAN(4) + DSTRAN(4)
!C
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C  END PACK
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!C
    END SUBROUTINE PHYSICS_EQUATIONS
