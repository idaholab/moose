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

      INTEGER LVL(4)
      DATA LVL/1,-1,-2,-3/

      PRINT *, "UEXTERNALDB", LOP, LRESTART, TIME(1), TIME(2),
     1    DTIME, KSTEP, KINC

C     TEST FETCHING OUTPUT DIRECTORY
      CALL GETOUTDIR(DIR, LEN)
      PRINT *, "getoutdir", LEN, DIR

C     TEST FETCHING JOB NAME
      CALL GETJOBNAME(DIR, LEN)
      PRINT *, "getjobname", LEN, DIR

      PRINT *, "Loading step is", KSTEP

      IF (KSTEP .GE. 1 .AND. KSTEP .LE. 4) THEN
C       TEST OUTPUTTING ERROR MESSAGES
        I(1) = 1
        I(2) = 2
        R(1) = 0.1
        R(2) = 0.2
        R(3) = 0.3
        C(1) = "ONE"
        C(2) = "TWO"
        CALL STDB_ABQERR(LVL(KSTEP),
     1      "Inline message %I %I %R %R %R %S %S!", I, R, C)

        MSG = "Fixed length message %I %I %R %R %R *%S* *%S*!"
        I(1) = 10
        I(2) = 20
        R(1) = 0.11
        R(2) = 0.22
        R(3) = 0.33
        C(1) = "TEN"
        C(2) = "TWENTY"
        CALL STDB_ABQERR(LVL(KSTEP), MSG, I, R, C)
      END IF

C     TEST MPI AND THREAD FUNCTIONS
      CALL GETNUMCPUS(N)
      PRINT *, "getnumprocs", N
      CALL GETRANK(N)
      PRINT *, "getrank", N

      CALL FLUSH()

      RETURN
      END
