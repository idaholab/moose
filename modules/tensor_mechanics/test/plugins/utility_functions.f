****************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
****************************************************************************************
****************************************************************************************
**
**
**
      SUBROUTINE UEXTERNALDB(LOP,LRESTART,TIME,DTIME,KSTEP,KINC)

C
C      INCLUDE 'ABA_PARAM.INC'
C
      DIMENSION TIME(2)

C ----------------------------------------------------------------
C     UEXTERNALDB FOR TESTING THE ABAQUS UTILITY FUNCTION INTERFACE
C ----------------------------------------------------------------

      CHARACTER(256) DIR
      INTEGER LEN

      CALL GETOUTDIR(DIR, LEN)
      PRINT *, LEN
      PRINT *, DIR

      RETURN
      END
