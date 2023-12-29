****************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING VOLUMETRIC SWELLING.                      **
****************************************************************************************
****************************************************************************************
**
**
**
      SUBROUTINE CREEP(DECRA,DESWA,STATEV,SERD,EC,ESW,P,QTILD,
     1     TEMP,DTEMP,PREDEF,DPRED,TIME,DTIME,CMNAME,LEXIMP,LEND,
     2     COORDS,NSTATV,NOEL,NPT,LAYER,KSPT,KSTEP,KINC)
C
C      INCLUDE 'ABA_PARAM.INC'
C
      CHARACTER*80 CMNAME
C
      REAL EPSDOT
      DIMENSION DECRA(5),DESWA(5),STATEV(*),PREDEF(*),DPRED(*),
     1     TIME(2),COORDS(*),EC(2),ESW(2)
C
C     DEFINE CONSTANTS
      EPSDOT=-0.001
C
      DESWA(1) = EPSDOT
C
      RETURN
      END
