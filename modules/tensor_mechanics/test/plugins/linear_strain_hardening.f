      INCLUDE 'SPRINCInterface.f'
      INCLUDE 'DROTCalc.f'
      INCLUDE 'ROTSIGInterface.f'

****************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ELASTO-VISCOPLASTICITY WITH LINEAR        **
**  ISOTROPIC HARDENING. LARGE DEFORMATION FORMULATION FOR PLANE STRAIN               **
**  AND AXI-SYMMETRIC ELEMENTS. IMPLICIT INTEGRATION WITH INITIAL STIFFNESS JACOBIAN  **
****************************************************************************************
****************************************************************************************
**
**
**
*USER SUBROUTINE
      SUBROUTINE UMAT(STRESS,STATEV,DDSDDE,SSE,SPD,SCD,
     1     RPL,DDSDDT,DRPLDE,DRPLDT,
     2     STRAN,DSTRAN,TIME,DTIME,TEMP,DTEMP,PREDEF,DPRED,CMNAME,
     3     NDI,NSHR,NTENS,NSTATV,PROPS,NPROPS,COORDS,DROT,PNEWDT,
     4     CELENT,DFGRD0,DFGRD1,NOEL,NPT,LAYER,KSPT,KSTEP,KINC)
C
C      INCLUDE 'ABA_PARAM.INC'
C
      CHARACTER*80 CMNAME
C
C
      DIMENSION STRESS(NTENS),STATEV(NSTATV),
     1     DDSDDE(NTENS,NTENS),DDSDDT(NTENS),DRPLDE(NTENS),
     2     STRAN(NTENS),DSTRAN(NTENS),TIME(2),PREDEF(1),DPRED(1),
     3     PROPS(NPROPS),COORDS(3),DROT(3,3),DFGRD0(3,3),DFGRD1(3,3)
C
C
      PARAMETER (ONE=1.0,TWO=2.0,THREE=3.0)
C
      DIMENSION DPSTRAN(6), STRESSOLD(6), DESTRAN(6), DSTRESS(6),
     +          PS(3), DIR(6), STRESSROT(NTENS)
C
C
C     PROPS(1) - E
C     PROPS(2) - NU
C     PROPS(3) - SYIELD
C     PROPS(4) - HARD
C--------------------------------------------------------------------
C     SPECIFY MATERIAL PROPERTIES
C
C
      E = PROPS(1)
      XNUE = PROPS(2)
      YIELD = PROPS(3)
      H = PROPS(4)
      ALPHA = 0.2418E-6
      BETA = 0.1135
C
C
C    RECOVER EFFECTIVE PLASTIC STRAIN, P, AND ISOTROPIC
C    HARDENING VARIABLE, R,FROM PREVIOUS TIME STEP
C
      P = STATEV(1)
      R = STATEV(2)
C
C
C    SET UP ELASTICITY MATRIX
C
      EBULK3 = E/(ONE-TWO*XNUE)
      XK = EBULK3/THREE
      EG2 = E/(ONE+XNUE)
      EG = EG2/TWO
      ELAM = (EBULK3-EG2)/THREE
C
C
C     ELASTIC STIFFNESS
C
      DO K1=1, NDI
         DO K2=1, NDI
            DDSDDE(K2, K1)=ELAM
         END DO
         DDSDDE(K1, K1)=EG2+ELAM
      END DO

      DO K1=NDI+1, NTENS
         DDSDDE(K1, K1)=EG
      END DO
C
C      PRINT *, DDSDDE
C     SAVE STRESS AT BEGINNING OF TIME STEP IN STRESSOLD
C
C      PRINT *, STRESS

      DO K=1,NTENS
         STRESSOLD(K) = STRESS(K)
      END DO
C
C     OBTAIN TRIAL (ELASTIC) STRESS
C      PRINT *, DSTRAN
C
      CALL KMLT1(DDSDDE,DSTRAN,DSTRESS,NTENS)
C
      DO K=1,NTENS
         STRESS(K) = STRESS(K) + DSTRESS(K)
      END DO
C
C      PRINT *, STRESS
      CALL SPRINC(STRESS,PS,1,NDI,NSHR)
C
C      PRINT *, PS
C    DETERMINE EFFECTIVE TRIAL STRESS
C
      PJ = (ONE/SQRT(TWO))*SQRT((PS(1)-PS(2))**2 +
     +     (PS(2)-PS(3))**2 + (PS(3)-PS(1))**2)
C      PRINT *, PJ
C
      FLOW = PJ - R - YIELD
C
C      PRINT *, FLOW

      STATEV(3) = FLOW

      XDP = 0.
      XFLOW = 1.
      XPHI = 1.
      XPHIDP = 1.
      XPHIR = 1.
      XRES = 1.

      IF(FLOW.GT.0.)THEN
C
C     USE NEWTON ITERATION TO DETERMINE EFFECTIVE PLASTIC
C     STRAIN INCREMENT
C
         R0 = R
         DO K=1,10
            XFLOW = BETA*(PJ-(THREE*EG*XDP)-R-YIELD)
            XPHI =  ALPHA*SINH(XFLOW)
            XPHIDP = -THREE*EG*ALPHA*BETA*COSH(XFLOW)
            XPHIR = -ALPHA*BETA*COSH(XFLOW)
            XRES = XPHI - (XDP/DTIME)
            DEQPL = XRES/((ONE/DTIME) - XPHIDP - (H*XPHIR))
            XDP = XDP + DEQPL
            R = R0 + H*XDP
            IF(ABS(XRES).LT.1.E-5) GOTO 10
         END DO
 10      CONTINUE
C
      END IF

C     DETERMINE THE INCREMENTS IN PLASTIC STRAIN (ENGINEERING SHEAR)
C
      SIGM = (ONE/THREE)*(STRESS(1) + STRESS(2) +
     +       STRESS(3))
C
C      PRINT *, SIGM
C      PRINT *, PJ
      IF(PJ.GT.1.E-10)THEN
         DO K=1,NDI
            DIR(K) = ((THREE/TWO)*(STRESS(K)-SIGM))/PJ
         END DO
         DO K=NDI+1,NTENS
            DIR(K) = ((THREE/TWO)*STRESS(K))/PJ
         END DO
      END IF
C      PRINT *, DIR
      DO K=1,NDI
         DPSTRAN(K) = XDP*DIR(K)
      END DO
      DO K=NDI+1,NTENS
         DPSTRAN(K) = TWO*XDP*DIR(K)
      END DO

C
C     CALCULATE THE ELASTIC STRAIN INCREMENTS
C
      DO K=1,NTENS
         DESTRAN(K)=DSTRAN(K)-DPSTRAN(K)
      END DO
C
C
C     DETERMINE STRESS INCREMENT
C
      CALL KMLT1(DDSDDE,DESTRAN,DSTRESS,NTENS)
C
C     UPDATE THE STRESS, EFFECTIVE PLASTIC STRAIN
C     (NOTE: ISOTROPIC HARDENING VARIABLE ALREADY UPDATED)
C
      DO K = 1,NTENS
         STRESS(K) = STRESSOLD(K) + DSTRESS(K)
      END DO

********************************************************
**    TESTING ROTSIG                                  **
********************************************************
      CALL DROTCALC(DROT, DFGRD0, DFGRD1, DTIME, NTENS)
      CALL ROTSIG(STRESS, DROT, STRESSROT, 1, NDI, NSHR)
C
      DO K =1,6
         STRESS(K) = STRESSROT(K)
      END DO

********************************************************
********************************************************
C
      P = P + XDP
C
C
C     STORE UPDATED STATE VARIABLES
C
      STATEV(1) = P
      STATEV(2) = R
C
C
      RETURN
      END
**
***************************************************
**         MULTIPLY (NTENS X NTENS) MATRIX WITH (NTENS X 1) VECTOR    *
***************************************************
*USER SUBROUTINE
      SUBROUTINE KMLT1(DM1,DM2,DM,NTENS)
C
C      INCLUDE 'ABA_PARAM.INC'
C
C
      DIMENSION DM1(NTENS,NTENS),DM2(NTENS),DM(NTENS)
C
      DO 10 I=1,NTENS
         X=0.0
         DO 20 K=1,NTENS
            Y=DM1(I,K)*DM2(K)
            X=X+Y
 20      CONTINUE
         DM(I)=X
 10   CONTINUE
      RETURN
      END
