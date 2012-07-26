****************************************************************************************
**  UTILITY SUBROUTINE FOR INTERFACE: SINV                                            **
**  CALULATE STRESS INVARIANTS                                                        **
**  EQUATIONS FROM ABAQUS SUBROUTINE MANUAL                                           **
**  SINV = (1/3)p                                                                     **
**  SINV2 = SQRT(1.5*Sij*Sij)=von mises criterion                                     **
****************************************************************************************
****************************************************************************************

      SUBROUTINE SINV(S, SINV1, SINV2, NDI, NSHR)

      INTEGER NDI, NSHR
      DIMENSION S(6)
      REAL SINV1, SINV2, P

      PARAMETER (ONE=1.D0,TWO=2.D0,THREE=3.D0, SIX = 6.D0,
     1     THIRD = ONE/THREE, SIXTH = ONE/SIX

C
C     CALCULATE FIRST INVARIANT
C
      P=THIRD*(S(1)+S(2)+S(3))
      SINV=P
C
C     CALCULATE SECOND INVARIANT
C
      SINV2=SQRT(((S(1)-S(2))**2+(S(2)-S(3))**2+(S(1)-S(3))**2 +
     1     6.*(S(4)**2+S(5)**2+S(6)**2))/2.)

      RETURN
      END
