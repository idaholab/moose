    SUBROUTINE UEL(RHS,AMATRX,SVARS,ENERGY,NDOFEL,NRHS,NSVARS,         &
    & PROPS,NPROPS,COORDS,MCRD,NNODE,U,DU,V,A,JTYPE,TIME,DTIME,        &
    & KSTEP,KINC,JELEM,PARAMS,NDLOAD,JDLTYP,ADLMAG,PREDEF,NPREDF,      &
    & LFLAGS,MLVARX,DDLMAG,MDLOAD,PNEWDT,JPROPS,NJPROP,PERIOD)
!C
    USE SHAPE_FUNCTIONS
    USE GAUSS_QUADRATURE
    USE CONSTANTS
    USE MATRIX_OPERATIONS
!C     
    IMPLICIT NONE
!C     
    REAL(R64), INTENT(OUT) :: RHS(NDOFEL,NRHS), AMATRX(NDOFEL,NDOFEL)
!C
    REAL(R64), INTENT(INOUT) :: SVARS(NSVARS), ENERGY(8), PNEWDT
!C
    REAL(R64), INTENT(IN) :: PROPS(NPROPS), COORDS(MCRD,NNODE)
    REAL(R64), INTENT(IN) :: U(NDOFEL), DU(NDOFEL)
    REAL(R64), INTENT(IN) :: V(NDOFEL), A(NDOFEL)
    REAL(R64), INTENT(IN) :: ADLMAG(MDLOAD), DDLMAG(MDLOAD)
    REAL(R64), INTENT(IN) :: PREDEF(2,NPREDF,NNODE)
    REAL(R64), INTENT(IN) :: PARAMS(3), TIME(2), DTIME, PERIOD
!C
    INTEGER, INTENT(IN) :: JPROPS(NJPROP), JDLTYP(MDLOAD), LFLAGS(5)
    INTEGER, INTENT(IN) :: NDOFEL, MLVARX, NRHS, NSVARS, NPROPS, NJPROP
    INTEGER, INTENT(IN) :: MCRD, NNODE, JTYPE, KSTEP, KINC, JELEM
    INTEGER, INTENT(IN) :: NDLOAD, MDLOAD, NPREDF
!C
    INTEGER, PARAMETER :: NSTATV = 10
    INTEGER, PARAMETER :: N = 3
!C
!C  node 1 = ux1 -  1, uy1 -  2, p1 -  3
!C  node 2 = ux2 -  4, uy2 -  5, p2 -  6
!C  node 3 = ux3 -  7, uy3 -  8, p3 -  9
!C  node 4 = ux4 - 10, uy4 - 11, p4 - 12
!C  node 5 = ux5 - 13, uy5 - 14
!C  node 6 = ux6 - 15, uy6 - 16
!C  node 7 = ux7 - 17, uy7 - 18
!C  node 8 = ux8 - 19, uy8 - 20, uz0 - 21
!C
    INTEGER, PARAMETER :: UX_ARRAY(8) = (/1, 4, 7, 10, 13, 15, 17, 19/)
    INTEGER, PARAMETER :: UY_ARRAY(8) = (/2, 5, 8, 11, 14, 16, 18, 20/)
    INTEGER, PARAMETER :: P_ARRAY(4)  = (/3, 6, 9, 12/)
    INTEGER, PARAMETER :: UZ_ARRAY(1) = (/21/)
!C
!C  Local Variables
    REAL(R64) :: RES(NDOFEL,1), JAC(NDOFEL,NDOFEL)
    REAL(R64) :: RES_INT(NDOFEL,1), JAC_INT(NDOFEL,NDOFEL) 
    REAL(R64) :: G_POINTS(N), G_WEIGHTS(N)
    REAL(R64) :: WEIGHT, X_POINT, Y_POINT, N_F(1,NNODE)
    REAL(R64) :: N_L(1,4), G_L(MCRD,4)
    REAL(R64) :: N_Q(1,8), G_Q(MCRD,8)
    REAL(R64) :: J(MCRD,MCRD), INV_J(MCRD,MCRD), DET_J
    REAL(R64) :: B_L(MCRD,4), B_Q(MCRD,8)
    REAL(R64) :: N1(1,NDOFEL), NT1(NDOFEL,1)
    REAL(R64) :: N2(1,NDOFEL), NT2(NDOFEL,1)
    REAL(R64) :: N3(1,NDOFEL), NT3(NDOFEL,1)
    REAL(R64) :: N4(1,NDOFEL), NT4(NDOFEL,1)
    REAL(R64) :: B1(MCRD,NDOFEL), BT1(NDOFEL,MCRD)
    REAL(R64) :: B2(MCRD,NDOFEL), BT2(NDOFEL,MCRD)
    REAL(R64) :: B3(MCRD,NDOFEL), BT3(NDOFEL,MCRD)
    REAL(R64) :: B4(1,NDOFEL), BT4(NDOFEL,1)
    REAL(R64) :: U1, U2, U3, P
    REAL(R64) :: DU1, DU2, DU3, DP
    REAL(R64) :: U1_OLD, U2_OLD, U3_OLD, P_OLD
    REAL(R64) :: GRAD_U1(MCRD), GRAD_U2(MCRD)
    REAL(R64) :: GRAD_DU1(MCRD), GRAD_DU2(MCRD) 
    REAL(R64) :: GRAD_U1_OLD(MCRD), GRAD_U2_OLD(MCRD)
    REAL(R64) :: GRAD_P(MCRD), GRAD_U3
    REAL(R64) :: GRAD_DP(MCRD), GRAD_DU3
    REAL(R64) :: GRAD_P_OLD(MCRD), GRAD_U3_OLD
    REAL(R64) :: LOCAL_DPRED(NPREDF), XY(8,MCRD)
    REAL(R64) :: LOCAL_PREDEF(NPREDF), LOCAL_STATV(NSTATV)
    REAL(R64) :: SECT_THICK, PRESSURE, NORMAL(MCRD), EDGE_SCALAR
!C
    INTEGER :: XINT, YINT, I, II, ERROR_FLAG, INP_NUMBER, JAC_INDEX
    INTEGER :: STAT_START, STAT_END, INT, LOAD, LOAD_TYPE
!C
    INTEGER, PARAMETER :: OUTPUT = 6
!C
    CALL GAUSS_WEIGHTS_LOCS(G_WEIGHTS,G_POINTS,N)
!C
    RES = ZERO
    JAC = ZERO
    INP_NUMBER = 0
!C
    XY = ZERO
    DO I = 1, 8
        DO II = 1, MCRD
            XY(I,II) = COORDS(II,I)
        END DO
    END DO
!C
    SECT_THICK = PROPS(9)
!C
!C  Domain Integration Loop
    DO YINT = 1, N
        DO XINT = 1, N
!C
            INP_NUMBER = INP_NUMBER + 1
            STAT_START = (INP_NUMBER-1)*NSTATV + 1
            STAT_END = STAT_START + NSTATV - 1
!C
!C          Guass Weights and Point
            X_POINT = G_POINTS(XINT)
            Y_POINT = G_POINTS(YINT)
            WEIGHT = G_WEIGHTS(XINT)*G_WEIGHTS(YINT)
!C
!C          Shape Functions
            CALL LINEAR_SHAPE_FUNCTION(N_L,G_L,X_POINT,Y_POINT)
            CALL QUAD_SHAPE_FUNCTION(N_Q,G_Q,X_POINT,Y_POINT)
            CALL FIELD_MAP_FUNCTION(N_F,X_POINT,Y_POINT)
!C
!C          Jacobian of Mapping
            J = matmul(G_Q,XY)
!C
            CALL MATRIX_INVERSE(J, DET_J, INV_J, MCRD, ERROR_FLAG)
!C
!C          Local Gradient
            B_L = matmul(INV_J,G_L)
            B_Q = matmul(INV_J,G_Q)
!C
            DO I = 1, NPREDF
                LOCAL_DPRED(I) = dot_product(N_F(1,:),PREDEF(2,I,:))
                LOCAL_PREDEF(I) = dot_product(N_F(1,:),PREDEF(1,I,:)) 
                LOCAL_PREDEF(I) = LOCAL_PREDEF(I) - LOCAL_DPRED(I)
            END DO
!C
!C          Field Shape Functions
            N1 = ZERO
            N2 = ZERO
            N3 = ZERO
            N4 = ZERO
            B1 = ZERO
            B2 = ZERO
            B3 = ZERO
            B4 = ZERO
!C
            DO I = 1, 8
                N1(1,UX_ARRAY(I)) = N_Q(1,I)
                B1(:,UX_ARRAY(I)) = B_Q(:,I)
                N2(1,UY_ARRAY(I)) = N_Q(1,I)
                B2(:,UY_ARRAY(I)) = B_Q(:,I)
            END DO
            DO I = 1, 4
                N3(1,P_ARRAY(I)) = N_L(1,I)
                B3(:,P_ARRAY(I)) = B_L(:,I)
            END DO
            N4(1,UZ_ARRAY(1)) = ONE
            B4(1,UZ_ARRAY(1)) = ONE/SECT_THICK
!C
            NT1 = transpose(N1)
            NT2 = transpose(N2)
            NT3 = transpose(N3)
            NT4 = transpose(N4)
            BT1 = transpose(B1)
            BT2 = transpose(B2)
            BT3 = transpose(B3)
            BT4 = transpose(B4)
!C
!C          Field Values and Gradients
            U1 = dot_product(N1(1,:),U)
            U2 = dot_product(N2(1,:),U)
            P = dot_product(N3(1,:),U)
            U3 = dot_product(N4(1,:),U)
!C
            DU1 = dot_product(N1(1,:),DU)
            DU2 = dot_product(N2(1,:),DU)
            DP = dot_product(N3(1,:),DU)
            DU3 = dot_product(N4(1,:),DU)
!C
            U1_OLD = U1 - DU1
            U2_OLD = U2 - DU2
            P_OLD = P - DP
            U3_OLD = U3 - DU3
!C
            GRAD_U1 = matmul(B1,U)
            GRAD_U2 = matmul(B2,U)
            GRAD_P = matmul(B3,U)
            GRAD_U3 = dot_product(B4(1,:),U)
!C
            GRAD_DU1 = matmul(B1,DU)
            GRAD_DU2 = matmul(B2,DU)
            GRAD_DP = matmul(B3,DU)
            GRAD_DU3 = dot_product(B4(1,:),DU)
!C
            GRAD_U1_OLD = GRAD_U1 - GRAD_DU1
            GRAD_U2_OLD = GRAD_U2 - GRAD_DU2
            GRAD_P_OLD = GRAD_P - GRAD_DP
            GRAD_U3_OLD = GRAD_U3 - GRAD_DU3
!C
            RES_INT = ZERO
            JAC_INT = ZERO
!C
!C          Unpack State Variables
            LOCAL_STATV = SVARS(STAT_START:STAT_END)
!C
            CALL PHYSICS_EQUATIONS(RES_INT, JAC_INT,                   &
    &                       U1, U2, U3, P,                             &
    &                       DU1, DU2, DU3, DP,                         &
    &                       U1_OLD, U2_OLD, U3_OLD, P_OLD,             &
    &                       GRAD_U1, GRAD_U2,                          &
    &                       GRAD_DU1, GRAD_DU2,                        &
    &                       GRAD_U1_OLD, GRAD_U2_OLD,                  &
    &                       GRAD_P, GRAD_U3, GRAD_DU3, GRAD_U3_OLD,    &
    &                       PROPS, NPROPS,                             &
    &                       LOCAL_PREDEF, LOCAL_DPRED, NPREDF,         &
    &                       N1, N2, N3, N4,                            &
    &                       NT1, NT2, NT3, NT4,                        &
    &                       B1, B2, B3, B4,                            &
    &                       BT1, BT2, BT3, BT4,                        &
    &                       DTIME, TIME, NDOFEL, MCRD,                 &
    &                       LOCAL_STATV, NSTATV)
!C
            RES = RES + WEIGHT*DET_J*RES_INT
            JAC = JAC + WEIGHT*DET_J*JAC_INT
!C
!C          Pack State Variables
            SVARS(STAT_START:STAT_END) = LOCAL_STATV
!C
        END DO
    END DO
!C
!C  Boundary Integration Loop
!C
    DO LOAD = 1, NDLOAD
        LOAD_TYPE = JDLTYP(LOAD)
        PRESSURE = ADLMAG(LOAD)
        DO INT = 1, N
!C
!C          Surface Definition
            SELECT CASE (LOAD_TYPE)
                CASE(1)
                    X_POINT = G_POINTS(INT)
                    Y_POINT = - ONE
                    JAC_INDEX = 1
                    EDGE_SCALAR = + ONE
                CASE(2)
                    X_POINT = + ONE
                    Y_POINT = G_POINTS(INT)
                    JAC_INDEX = 2
                    EDGE_SCALAR = + ONE
                CASE(3)
                    X_POINT = G_POINTS(INT)
                    Y_POINT = + ONE
                    JAC_INDEX = 1
                    EDGE_SCALAR = - ONE
                CASE(4)
                    X_POINT = - ONE
                    Y_POINT = G_POINTS(INT)
                    JAC_INDEX = 2
                    EDGE_SCALAR = - ONE
            END SELECT
!C
!C          Guass Weights and Point
            WEIGHT = G_WEIGHTS(INT)
!C    
!C          Shape Functions
            CALL QUAD_SHAPE_FUNCTION(N_Q,G_Q,X_POINT,Y_POINT)
!C    
!C          Jacobian of Mapping
            J = matmul(G_Q,XY)
!C
            DET_J = sqrt(J(JAC_INDEX,1)**2 + J(JAC_INDEX,2)**2)
!C    
!C          Field Shape Functions
            N1 = ZERO
            N2 = ZERO
            N4 = ZERO
!C    
            DO I = 1, 8
                N1(1,UX_ARRAY(I)) = N_Q(1,I)
                N2(1,UY_ARRAY(I)) = N_Q(1,I)
            END DO
            N4(1,UZ_ARRAY(1)) = ONE
!C    
            NT1 = transpose(N1)
            NT2 = transpose(N2)
!C
            NORMAL(1) = + EDGE_SCALAR*J(JAC_INDEX,2)/(DET_J+TINY)
            NORMAL(2) = - EDGE_SCALAR*J(JAC_INDEX,1)/(DET_J+TINY)
!C
            RES_INT = NT1*PRESSURE*NORMAL(1)
            RES_INT = NT2*PRESSURE*NORMAL(2)
!C    
            RES = RES + WEIGHT*DET_J*RES_INT
!C    
        ENDDO
    ENDDO
!C
!C
    RHS(1:NDOFEL,1) = - RES(:,1)
    AMATRX = + JAC
!C
    WRITE(OUTPUT,'(A24)') '*Solution'
    WRITE(OUTPUT,'(3(ES24.16,","))') U(1:3*4)
    WRITE(OUTPUT,'(2(ES24.16,","))') U(3*4+1:21)
    WRITE(OUTPUT,'(A24)') '*State Variables'
    WRITE(OUTPUT,'(10(ES24.16,","))') SVARS
!C
    END SUBROUTINE UEL
