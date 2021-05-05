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
      DOUBLE PRECISION SINV1, SINV2, P, S(6)

      PARAMETER (ONE=1.D0,TWO=2.D0,THREE=3.D0, SIX = 6.D0,
     1     THIRD = ONE/THREE, SIXTH = ONE/SIX
C
C======================================================================+
C-----------
C  INPUT :
C-----------
C  S     	: STRESS TENSOR
C  NSHR 	: NUMBER OF SHEAR COMPONENTS
C  NDI  	: NUMBER OF DIRECT COMPONENTS
C-----------
C  OUTPUT :
C-----------
C  SINV1	: FIRST INVARIANT
C  SINV2        : SECOND INVARIANT
C----------------------------------------------------------------------+
C=======================================================================
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
