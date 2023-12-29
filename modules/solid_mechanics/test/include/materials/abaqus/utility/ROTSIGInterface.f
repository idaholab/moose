****************************************************************************************
**  UTILITY SUBROUTINE FOR INTERFACE: ROTSIG                                          **
**  CALULATE ROTATED STRESS/STRAIN TENSOR                                             **
****************************************************************************************
**  ROTATED STRESS CALCULATIONS BASED OFF OF "ON THE ROTATED STRESS TENSOR AND THE    **
**  MATERIAL VERSION OF THE DOYLE-ERICKSEN FORMULA" BY J. SIMO AND J. MARSDEN         **
****************************************************************************************

      SUBROUTINE ROTSIG (S, DROT, SPRIME, LSTR, NDI, NSHR)

      DOUBLE PRECISION S(NDI+NSHR), R(3,3), RINV(3,3), SPRIME(NDI+NSHR),
     1     DROT(3,3)
      INTEGER LSTR, NDI, NSHR
C
C======================================================================+
C-----------
C  INPUT :
C-----------
C  DROT  	: ROTATION INCREMENT MATRIX
C  S     	: STRESS OR STRAIN TENSOR TO BE ROTATED
C  LSTR 	: FLAG DETERMINING STRESS OR STRAIN CALCULATION
C  NDI  	: NUMBER OF DIRECT STRESS/STRAIN COMPONENTS
C-----------
C  OUTPUT :
C-----------
C  SPRIME	: ROTATED STRESS/STRAIN TENSOR
C----------------------------------------------------------------------+
C=======================================================================

C     STRESS = ROTATIONMATRIX * ROTATEDSTRESS
C     ROTATEDSTRESS = ROTATIONMATRIX^-1 * STRESS
C
      RXX = DROT(1,1)
      RXY = DROT(2,1)
      RXZ = DROT(3,1)
      RYX = DROT(1,2)
      RYY = DROT(2,2)
      RYZ = DROT(3,2)
      RZX = DROT(1,3)
      RZY = DROT(2,3)
      RZZ = DROT(3,3)
C
C     Calculate Determinant of the rotation matrix
C

      DETR = (RXX*RYY*RZZ)+(RXY*RYZ*RZX)+(RXZ*RYX*RZY)-
     +     (RZX*RYY*RXZ)-(RZY*RYZ*RXX)-(RZZ*RYX*RXY)
C
C     Inverse of determinant
C
      IF (DETR.NE. 0.0) THEN
         DETINV = 1. / DETR
C
C     Calculate inverse rotation matrix components
C
         RINV(1,1) = +(RYY*RZZ-RZY*RYZ) * DETINV
         RINV(2,1) = -(RXY*RZZ-RZY*RXZ) * DETINV
         RINV(3,1) = +(RXY*RYZ-RYY*RXZ) * DETINV
         RINV(1,2) = -(RYX*RZZ-RZX*RYZ) * DETINV
         RINV(2,2) = +(RXX*RZZ-RZX*RXZ) * DETINV
         RINV(3,2) = -(RXX*RYZ-RYX*RXZ) * DETINV
         RINV(1,3) = +(RYX*RZY-RZX*RYY) * DETINV
         RINV(2,3) = -(RXX*RZY-RZX*RXY) * DETINV
         RINV(3,3) = +(RXX*RYY-RYX*RXY) * DETINV

C
C     Calculate rotated stress/strain tensor based on LSTR value
C     LSTR = 1 for stresses
C     LSTR = 2 for strains - Equations must be added
C
         IF ((LSTR.EQ.1).OR.(LSTR.EQ.2)) THEN
            SPRIME(1) = RINV(1,1)*S(1)+RINV(2,1)*S(4)+RINV(3,1)*S(6)
            SPRIME(2) = RINV(1,2)*S(4)+RINV(2,2)*S(2)+RINV(3,2)*S(5)
            SPRIME(3) = RINV(1,3)*S(6)+RINV(2,3)*S(5)+RINV(3,3)*S(3)
            SPRIME(4) = RINV(1,1)*S(4)+RINV(2,1)*S(2)+RINV(3,1)*S(5)
            SPRIME(5) = RINV(1,2)*S(6)+RINV(2,2)*S(5)+RINV(3,2)*S(3)
            SPRIME(6) = RINV(1,1)*S(6)+RINV(2,1)*S(5)+RINV(3,1)*S(3)
         ELSE
            DO K = 1, NDI+NSHR
               SPRIME(K) = S(K)
            END DO
         END IF
      ELSE
         DO K = 1, NDI+NSHR
            SPRIME(K) = S(K)
         END DO
      END IF

      RETURN
      END
