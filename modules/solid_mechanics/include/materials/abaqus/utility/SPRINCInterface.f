****************************************************************************************
**  UTILITY SUBROUTINE FOR INTERFACE: SPRINC                                          **
**  CALULATE PRINCIPAL VALUES                                                         **
**  EQUATIONS FROM: ADVANCED STRENGTH AND APPLIED ELASTICITY: FOURTH EDITION          **
**  AUTHORS: ANSEL C. UGURAL, SAUL K. FENSTER PG. 505 APPENDIX B1                     **
****************************************************************************************
****************************************************************************************

      SUBROUTINE SPRINC(S, PS, LSTR, NDI, NSHR)

      INTEGER LSTR, NDI, NSHR
      DIMENSION S(6), PS(3)
      REAL S, I1, I2, I3, J1, J2, J3, R, T, Q, ARG, ALP, SCALC, 
     1     PRINC1, PRINC2, PRINC3

      PARAMETER (ONE=1.D0,TWO=2.D0,THREE=3.D0, TWOSEVEN = 27.D0,
     1     THIRD = ONE/THREE, TWENTYSEVENTH= ONE/TWOSEVEN,
     2     TWOTWENTYSEVENTH = TWO/TWOSEVEN, ONETWENTYDEG=2.094395102D0,
     3     TWOFORTYDEG=4.188790205D0)

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
         ARG = -Q/(TWO*T)
         IF (ARG.GT.1) THEN
            ARG = ARG - 1.E-10
         END IF
         ALP = ACOS(ARG)
         SCALC = SQRT(THIRD*R)
         
         PRINC1 = (2*SCALC*COS(ALP/THREE))+(THIRD*I1)
         PRINC2 = (2*SCALC*COS((ALP/THREE)+TWOFORTYDEG))+(THIRD*I1)
         PRINC3 = (2*SCALC*COS((ALP/THREE)+ONETWENTYDEG))+(THIRD*I1)

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
