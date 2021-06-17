****************************************************************************************
**  UTILITY SUBROUTINE FOR INTERFACE: SPRINC                                          **
**  CALULATE PRINCIPAL VALUES AND DIRECTION COSINES                                   **
**  EQUATIONS FROM: ADVANCED STRENGTH AND APPLIED ELASTICITY: FOURTH EDITION          **
**  ANSEL C. UGURAL, SAUL K. FENSTER PG. 505 APPENDIX B1 AND B2                       **
****************************************************************************************
****************************************************************************************

      SUBROUTINE SPRIND(S, PS, AN, LSTR, NDI, NSHR)

      INTEGER LSTR, NDI, NSHR
      DIMENSION S(6), PS(3), A(3), B(3), C(3), V(3), AN(3,3)
      DOUBLE PRECISION I1, I2, I3, J1, J2, J3, R, T, Q, ALP, SCALC,
     1     PRINC1, PRINC2, PRINC3, L(3), M(3), N(3)
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
C  PS(I), I=1,2,3       : THE THREE PRINCIPAL VALUES
C  AN(K1,I), I=1,2,3	: THE DIRECTION COSINES OF THE PRINCIPAL DIRECTIONS CORRESPONDING TO PS(K1)
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

C     Assign Principal Stress/Strains values to array
C
      PS(1) = PRINC1
      PS(2) = PRINC2
      PS(3) = PRINC3

C
C     Calculate cofactors and factor
C
      DO K=1, 3
         A(K)=((S(2)-S(K))*(S(3)-S(K)))-(S(5)**2)
         B(K)=-(S(4)*(S(3)-S(K)))-(S(5)*S(6))
         C(K)=(S(4)*S(5))-((S(2)-S(K))*S(6))
      END DO

      DO K=1, 3
         V(K)=1/SQRT(A(K)**2+B(K)**2+C(K)**2)
      END DO
C
C     Calculate Direction Cosines
C
      DO K=1, 3
         L(K)=A(K)*V(K)
         M(K)=B(K)*V(K)
         N(K)=C(K)*V(K)
      END DO

C
C     Assign Direction Cosines to array locations
C
      AN(1,1)=L(1)
      AN(1,2)=M(1)
      AN(1,3)=N(1)
      AN(2,1)=L(3)
      AN(2,2)=M(3)
      AN(2,3)=N(3)
      AN(3,1)=L(2)
      AN(3,2)=M(2)
      AN(3,3)=N(2)

      RETURN
      END
