****************************************************************************************
**  UTILITY SUBROUTINE FOR INTERFACE: SPRINC                                          **
**  CALULATE PRINCIPAL VALUES                                                         **
**  EQUATIONS FROM: ADVANCED STRENGTH AND APPLIED ELASTICITY: FOURTH EDITION          **
**  ANSEL C. UGURAL, SAUL K. FENSTER PG. 505 APPENDIX B1                              **
****************************************************************************************
****************************************************************************************

      SUBROUTINE SPRINC(S, PS, LSTR, NDI, NSHR)

      INTEGER LSTR, NDI, NSHR
      DIMENSION S(6), PS(3)
      DOUBLE PRECISION S, I1, I2, I3, J1, J2, J3, R, T, Q, ARG, ALP,
     1     SCALC, PRINC1, PRINC2, PRINC3

      PARAMETER (ONE=1.D0,TWO=2.D0,THREE=3.D0, TWOSEVEN = 27.D0,
     1     THIRD = ONE/THREE, TWENTYSEVENTH= ONE/TWOSEVEN,
     2     TWOTWENTYSEVENTH = TWO/TWOSEVEN, ONETWENTYDEG=2.094395102D0,
     3     TWOFORTYDEG=4.188790205D0)
C
C======================================================================+
C-----------
C  INPUT :
C-----------
C  S     	: STRESS OR STRAIN TENSOR
C  LSTR 	: FLAG DETERMINING STRESS OR STRAIN CALCULATION
C  NDI  	: NUMBER OF DIRECT STRESS/STRAIN COMPONENTS
C  NSHR         : NUMBER OF SHEAR COMPONENTS
C-----------
C  OUTPUT :
C-----------
C  PS(I)	: THE THREE PRINCIPAL VALUES
C----------------------------------------------------------------------+
C=======================================================================
C
C     Calculate stress or strain invariants based on LSTR value

      IF (LSTR.EQ.1) THEN
         I1 = S(1)+S(2)+S(3)
         I2 = (S(1)*S(2))+(S(2)*S(3))+(S(1)*S(3))
     1        -(S(4)**2)-(S(5)**2)-(S(6)**2)
         I3 = (S(1)*S(2)*S(3))+(2*S(4)*S(5)*S(6))-(S(1)*S(5)**2)
     1        -(S(2)*S(6)**2)-(S(3)*S(4)**2)

         R = (THIRD*I1**2)-I2
         T = SQRT(TWENTYSEVENTH*R**3)
         Q = (THIRD*I1*I2)-I3-(TWOTWENTYSEVENTH*I1**3)
C         PRINT *, Q
         IF (T.LT.1.) THEN
            PRINC1=0
            PRINC2=0
            PRINC3=0
         ELSE
            ARG = -Q/(TWO*T)
            IF (ARG.GT.1.) THEN
               ARG = ARG - 1.E-10
            ELSE IF (ARG.LT.-1.) THEN
               ARG = ARG + 1.E-10
            END IF

            ALP = ACOS(ARG)
C            PRINT *, ALP
            SCALC = SQRT(THIRD*R)
C            PRINT *, SCALC
            PRINC1 = (2*SCALC*COS(ALP/THREE))+(THIRD*I1)
            PRINC2 = (2*SCALC*COS((ALP/THREE)+TWOFORTYDEG))+(THIRD*I1)
            PRINC3 = (2*SCALC*COS((ALP/THREE)+ONETWENTYDEG))+(THIRD*I1)

         END IF

      ELSE
         J1 = S(1)+S(2)+S(3)
         J2 = S(1)*S(2)+S(2)*S(3)+S(1)*S(3)
     1        -(S(4)**2)-(S(5)**2)-(S(6)**2)
         J3 = S(1)*S(2)*S(3)+(2*S(4)*S(5)*S(6))-(S(1)*S(5)**2)
     1        -(S(2)*S(6)**2)-(S(3)*S(4)**2)
         R = (THIRD*J1**2)-I2
         T = SQRT(TWENTYSEVENTH*R**3)
         Q = (THIRD*J1*J2)-J3-(TWOTWENTYSEVENTH*J1**3)
         ARG = -Q/(TWO*T)
         IF (ARG.GT.1) THEN
            ARG = ARG - 1.E-10
         END IF
         ALP = ACOS(ARG)
         SCALC = SQRT(THIRD*R)


         PRINC1 = (2*SCALC*COS(ALP/THREE))+(THIRD*J1)
         PRINC2 = (2*SCALC*COS((ALP/THREE)+TWOFORTYDEG))+(THIRD*J1)
         PRINC3 = (2*SCALC*COS((ALP/THREE)+ONETWENTYDEG))+(THIRD*J1)

      END IF
C
C     Assign Principal Stress/Strains values to array
C

      PS(1) = PRINC1
      PS(2) = PRINC2
      PS(3) = PRINC3

      RETURN
      END
