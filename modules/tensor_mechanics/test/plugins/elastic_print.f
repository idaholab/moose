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
      CHARACTER*80 CMNAME
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

C     CALCULATE STRESS, ELASTIC STRAIN AND THERMAL STRAIN
C     compute stress using a stress increment from a strain increment
C
      DO K1=1, NTENS
         DO K2=1, NTENS
            STRESS(K1)=STRESS(K1)+DDSDDE(K2, K1)*DSTRAN(K2)
         END DO
      END DO

C
C     PRINTING FOR VERIFICATION PURPOSES
C
      IF (TIME(1).GT.1.D0 .AND. COORDS(1).GT.0.25D0 .AND.
     1   COORDS(2).GE.0.5D0 .AND. COORDS(3).GT.0.25D0) THEN
        DO K1=1, NTENS
          WRITE(*,120)  K1, STRAN(K1)
          WRITE(*,125)  K1, DSTRAN(K1)
        END DO

        DO K1=1, 3
          WRITE(*,130) K1, COORDS(K1)
        END DO

        DO K1=1, NDI
          DO K2=1, NDI
            WRITE(*,165) K1, K2, DFGRD0(K1, K2)
          END DO
        END DO

        DO K1=1, NDI
          DO K2=1, NDI
            WRITE(*,170) K1, K2, DFGRD1(K1, K2)
          END DO
        END DO

        DO K1=1, NDI
          DO K2=1, NDI
            WRITE(*,175) K1, K2, DROT(K1, K2)
          END DO
        END DO

        WRITE(*,135) 1, TIME(1)
        WRITE(*,135) 2, TIME(2)
        WRITE(*,140) CELENT
        WRITE(*,*) 'CMNAME: ',CMNAME
        WRITE(*,145) NDI
        WRITE(*,150) NSHR
        WRITE(*,155) NTENS
        WRITE(*,160) NOEL
        WRITE(*,162) NPT
        WRITE(*,180) LAYER
        WRITE(*,185) KSPT
        WRITE(*,190) KSTEP
        WRITE(*,195) KINC
        CALL FLUSH()
      ENDIF


120   FORMAT ( 1X 'STRAIN_', I2, :, 3X, 4F10.7 )
125   FORMAT ( 1X 'DSTRAIN_', I2, :, 3X, 4F10.7 )
130   FORMAT ( 1X 'COORDS_', I2, :, 3X, 4F10.7 )
135   FORMAT ( 1X 'TIME_', I2, :, 3X, 4F10.7 )
140   FORMAT ( 1X 'CELENT', :,  2F10.7 )
145   FORMAT ( 1X 'NDI_', :,  2I2 )
150   FORMAT ( 1X 'NSHR_', :,  2I2 )
155   FORMAT ( 1X 'NTENS_', :,  2I2 )
160   FORMAT ( 1X 'NOEL_', :,  2I2 )
162   FORMAT ( 1X 'NPT_', :,  2I2 )
165   FORMAT ( 1X 'DFGRD0_', I2, I2,:, 3X, 4F10.7 )
170   FORMAT ( 1X 'DFGRD1_', I2, I2,:, 3X, 4F10.7 )
175   FORMAT ( 1X 'DROT_', I2, I2,:, 3X, 4F10.7 )
180   FORMAT ( 1X 'LAYER_', :,  2I2 )
185   FORMAT ( 1X 'KSPT_', :,  2I2 )
190   FORMAT ( 1X 'KSTEP_', :,  2I2 )
195   FORMAT ( 1X 'KINC_', :,  2I2 )

      RETURN
      END
