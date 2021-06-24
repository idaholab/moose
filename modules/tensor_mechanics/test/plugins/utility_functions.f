****************************************************************************************
**  UEXTERNALDB FOR TESTING THE ABAQUS UTILITY FUNCTION INTERFACE                      **
****************************************************************************************
****************************************************************************************
**
**
      SUBROUTINE UEXTERNALDB(LOP,LRESTART,TIME,DTIME,KSTEP,KINC)

C
C      INCLUDE 'ABA_PARAM.INC'
C
      DIMENSION TIME(2)
      REAL*8 DTIME

      CHARACTER(256) DIR
      INTEGER LEN
      CHARACTER*80 MSG
      REAL*8 R(3)
      INTEGER I(2)
      CHARACTER*8 C(2)
      INTEGER N

      PRINT *, "UEXTERNALDB", LOP, LRESTART, TIME(1), TIME(2),
     1    DTIME, KSTEP, KINC

C     TEST FETCHING OUTPUT DIRECTORY
      CALL GETOUTDIR(DIR, LEN)
      PRINT *, "getoutdir", LEN, DIR

C     TEST OUTPUTTING ERROR MESSAGES
      I(1) = 1
      I(2) = 2
      R(1) = 0.1
      R(2) = 0.2
      R(3) = 0.3
      C(1) = "ONE"
      C(2) = "TWO"
      CALL STDB_ABQERR(1, "Inline message %I %I %R %R %R %S %S!",
     1    I, R, C)

      MSG = "Fixed length message %I %I %R %R %R *%S* *%S*!"
C      I(1) = 10
C      I(2) = 20
C      R(1) = 0.11
C      R(2) = 0.22
C      R(3) = 0.33
C      C(1) = "TEN"
C      C(2) = "TWENTY"
C      CALL STDB_ABQERR(1, MSG, I, R, C)

C     TEST MPI AND THREAD FUNCTIONS
C      CALL GETNUMCPUS(N)
C      PRINT *, "getnumprocs", N
C      CALL GETRANK(N)
C      PRINT *, "getrank", N

      RETURN
      END
