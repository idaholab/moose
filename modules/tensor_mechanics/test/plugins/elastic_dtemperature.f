****************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
****************************************************************************************
****************************************************************************************
**
**
**
      SUBROUTINE UMAT(STRESS,STATEV,DDSDDE,SSE,SPD,SCD,
     1     RPL,DDSDDT,DRPLDE,DRPLDT,
     2     STRAN,DSTRAN,TIME,DTIME,TEMP,DTEMP,PREDEF,DPRED,CMNAME,
     3     NDI,NSHR,NTENS,NSTATV,PROPS,NPROPS,COORDS,DROT,PNEWDT,
     4     CELENT,DFGRD0,DFGRD1,NOEL,NPT,LAYER,KSPT,KSTEP,KINC)

C
C      INCLUDE 'ABA_PARAM.INC'
C
C      CHARACTER*8 CMNAME
C
      DIMENSION STRESS(NTENS),STATEV(NSTATV),DDSDDE(NTENS, NTENS),
     1     DDSDDT(6),DRPLDE(6),STRAN(6),DSTRAN(NTENS),
     2     PREDEF(1),DPRED(1),PROPS(NPROPS),COORDS(3),DROT(3,3),
     3     DFGRD0(3,3), DFGRD1(3,3), TIME(2), DELDSE(6,6)

      PARAMETER(ZERO=0.D0, ONE=1.D0, TWO=2.D0, THREE=3.D0)

C ----------------------------------------------------------------
C
C     UMAT FOR ISOTROPIC ELASTICITY
C
C     CANNOT BE USED FOR PLANE STRESS
C ----------------------------------------------------------------
C
C      PROPS(1) - E
C
C      PROPS(2) - NU
C ----------------------------------------------------------------
C
C      IF (NDI.NE.3) THEN
C         WRITE (7, *) ’THIS UMAT MAY ONLY BE USED FOR ELEMENTS
C    1        WITH THREE DIRECT STRESS COMPONENTS’
C         CALL XIT
C      ENDIF
C
C     ELASTIC PROPERTIES AT START OF INCREMENT
C
      EMOD=PROPS(1)
      ENU=PROPS(2)
C     TEMPERATURE DEFINED AT THE END OF THE STEP
      EMOD=EMOD*273.D0/(TEMP+DTEMP+DTEMP)
      EBULK3=EMOD/(ONE-TWO*ENU)
      EG2=EMOD/(ONE+ENU)
      EG=EG2/TWO
      ELAM=(EBULK3-EG2)/THREE

C
C     ELASTIC STIFFNESS AT END OF INCREMENT AND STIFFNESS CHANGE
C
      DO K1=1, NDI
         DO K2=1, NDI
            DDSDDE(K2, K1)=ELAM
         END DO
         DDSDDE(K1, K1)=EG2+ELAM
      END DO
      DO K1=NDI+1, NTENS
         DDSDDE(K1 ,K1)=EG
      END DO

C
C     CALCULATE STRESS, ELASTIC STRAIN AND THERMAL STRAIN
C     compute stress using the total strain
C
      DO K1=1, NTENS
         STRESS(K1)=0
      END DO
      DO K1=1, NTENS
         DO K2=1, NTENS
            STRESS(K1)=STRESS(K1)+DDSDDE(K2, K1)*(STRAN(K2)+DSTRAN(K2))
         END DO
      END DO

      RETURN
      END
