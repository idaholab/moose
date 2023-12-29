      SUBROUTINE DROTCALC(DROT ,DFGRD0,DFGRD1,DTIME,NTENS)
C======================================================================+
C----------------------------------------------------------------------+
C----------------  CALCULATION OF ROTATION INCREMENT  -----------------+
C--------------------  FOR THE JAUMANN DERIVATIVE  --------------------+
C----------------------  METHOD OF HUGHES-WINGET  ---------------------+
C----------------------------------------------------------------------+
C======================================================================+
C-----------
C  INPUT :
C-----------
C  DFGRD0	: DEFORMATION GRADIENT AT TIME T
C  DFGRD1	: DEFORMATION GRADIENT AT TIME T+DT
C  DTIME	: TIME INCREMENT
C  NTENS	: NUMBER OF TENSOR COMPONENTS
C  M		: COMMENT CHECK PARAMETER - NOT USED
C  MPI		: UNIT FILE - NOT USED
C-----------
C  OUTPUT :
C-----------
C  DROT		: ROTATION INCEMENT MATRIX
C----------------------------------------------------------------------+
C=======================================================================
C----------------------------------------------------------------------+
C.1-----  Precision
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C.2-----  Parameter
      PARAMETER (N=3)
C.3-----  Dimension
      DIMENSION DFGRD0(3,3),DFGRD1(3,3),DROT(3,3),DF(3,3),F0I(3,3)
      DIMENSION DQDQ(3,3),DFM(3,3),DFP(3,3)
      DIMENSION VL(3,3),DW(3,3),DM(3,3),DP(3,3)
      DIMENSION TDR(3,3),TI(3,3)
C.4-----  Real,Integer,Complex,Double precision,Logical,Character
C.5-----  Common
C.6-----  Equivalence
C.7-----  Save,Data
      DATA HALF,ONE,TWO/0.5D0,1.D0,2.D0/
C.8-----  Functions Definition
C----------------------------------------------------------------------+
C======================================================================+
C----------------------------------------------------------------------+
C-------  Deformation Gradient Increment DF
C
      CALL AFFECT(N*N,DFGRD0,F0I)
      CALL INVERS(F0I,N,N,DET)
      CALL PRDMAT(N,DFGRD1,F0I,DF)
C----------------------------------------------------------------------+
C----------------------------------------------------------------------+
C-------  Velocity Gradient L(t+Dt/2)
C
      DO I=1,N
         DO J=1,N
            DFM(I,J) = DF(I,J)
            DFP(I,J) = DF(I,J)
         END DO
         DFM(I,I) = DFM(I,I)-ONE
         DFP(I,I) = DFP(I,I)+ONE
      END DO
C-------
      CALL INVERS(DFP,N,N,DET)
      CALL PRDMAT(N,DFM,DFP,VL)
C-------
      TWODT = TWO/DTIME
      DO I=1,N
         DO J=1,N
            VL(I,J) = TWODT*VL(I,J)
         END DO
      END DO
C-------
      DO I=1,N
         DO J=1,N
            DW(I,J) = HALF*(VL(I,J)-VL(J,I))
         END DO
      END DO
C----------------------------------------------------------------------+
C-------  Increment Rotation Matrix DROT (Jaumann)
C
      HALFDT = HALF*DTIME
      DO I=1,N
         DO J=1,N
            DP(I,J) = HALFDT*DW(I,J)
            DM(I,J) = -DP(I,J)
         END DO
         DP(I,I) = DP(I,I)+ONE
         DM(I,I) = DM(I,I)+ONE
      END DO
C-------
      CALL INVERS(DM,N,N,DET)
      CALL PRDMAT(N,DM,DP,DROT)
C----------------------------------------------------------------------+
C======================================================================+
C----------------------------------------------------------------------+
C---------   FORMATS  -------------------------------------------------+
C----------------------------------------------------------------------+
 101  FORMAT(1X,3(D18.12,2X))
C----------------------------------------------------------------------+
C======================================================================+
      RETURN
      END
C
      SUBROUTINE INVERS(A,N,NP,DET)
C======================================================================
C---------------------------------------------------------------------+
C--------------  INVERSION OF A NON-SYMMETRIC MATRIX  ----------------+
C---------------------------  WITH SEARCH  ---------------------------+
C----------------  OF A NON-ZERO PIVOT ON A COLUMN  ------------------+
C---------------------------------------------------------------------+
C------------------  SUBROUTINE ISSUED FROM BOOK  --------------------+
C----------------  OF GOURI DHATT AND GILBERT TOUZOT  ----------------+
C=====================================================================+
C-------------
C  INPUT :
C-------------
C  A		: MATRIX TO INVERSE
C  N		: DIMENSION OF MATRIX TO INVERSE
C  NP		: DIMENSION OF MATRIX IN CALLED SUBROUTINE
C-------------
C  LOCAL :
C-------------
C  K		: WORK VECTOR INTEGER
C-------------
C  OUPTUT :
C-------------
C  A		: INVERSED MATRIX
C  DET		: MATRIX DETERMINANT
C=====================================================================+
C---------------------------------------------------------------------+
C=====================================================================+
C.1-----  Implicit, External
      IMPLICIT DOUBLE  PRECISION (A-H,O-Z)
C.2-----  Parameter
C.3-----  Dimension
      DIMENSION A(NP,NP),K(6)
C.4-----  Real,Integer,Complex,Double precision,Logical,Character
C.5-----  Common
C.6-----  Equivalence
C.7-----  Save,Data
      DATA ZERO,ONE,EPS/0.D0,1.D0,1.D-13/
C.8-----  Funtions Definition
C---------------------------------------------------------------------+
C======================================================================
C---------------------------------------------------------------------+
C-------  Initialization
C
      DET=ONE
      DO I=1,N
         K(I)=I
      END DO
C---------------------------------------------------------------------+
C-------  Start of Matrix Inversion A
C
      DO II=1,N
C-----------------------------------------------------------+
C-------  Search non-zero pivot on column II
         DO I=II,N
            XPIV=A(I,II)
            IF(DABS(XPIV).GT.EPS) GO TO 10
         END DO
         DET=ZERO
         GOTO 1000
C-----------------------------------------------------------+
C-------  Switch line II with line I
 10      DET=DET*XPIV
         IF(I.EQ.II) GO TO 20
         I1=K(II)
         K(II)=K(I)
         K(I)=I1
         DO J=1,N
            C=A(I,J)
            A(I,J)=A(II,J)
            A(II,J)=C
         END DO
         DET=-DET
C-----------------------------------------------------------+
C-------  Normalize pivot line
 20      C=ONE/XPIV
         A(II,II)=ONE
         DO J=1,N
            A(II,J)=A(II,J)*C
         END DO
C-----------------------------------------------------------+
C-------  Elimination
         DO I=1,N
            IF(I.EQ.II) GO TO 30
            C=A(I,II)
            A(I,II)=ZERO
            DO J=1,N
               A(I,J)=A(I,J)-C*A(II,J)
            END DO
 30         CONTINUE
         END DO
      END DO
C---------------------------------------------------------------------+
C
C-------  Reorder columns of the inversed matrix
      DO J=1,N
C-------  Search J1 such as K(J1)=J
         DO J1=J,N
            JJ=K(J1)
            IF(JJ.EQ.J) GO TO 100
         END DO
100      IF(J.EQ.J1) GO TO 110
C-------  Switch columns J and J1
         K(J1)=K(J)
         DO I=1,N
            C=A(I,J)
            A(I,J)=A(I,J1)
            A(I,J1)=C
         END DO
110      CONTINUE
      END DO
C
C---------------------------------------------------------------------+
1000  CONTINUE
C---------------------------------------------------------------------+
C---------------------------------------------------------------------+
 101  FORMAT(1X,6(D12.5,2X))
 102  FORMAT(1X,A20,6(D12.5,2X))
C---------------------------------------------------------------------+
C=====================================================================+
      RETURN
      END
C
      SUBROUTINE PRDMAT(NTENS,VM1,VM2,VM3)
C======================================================================
C----------------------------------------------------------------------
C-----------------------  PRODUCT OF TWO MATRICES  --------------------
C----------------------------------------------------------------------
C======================================================================
C-------------
C  INPUT :
C-------------
C  VM1		: MATRIX OR FOURTH ORDER TENSOR (NDIM1,NDIM2)
C  VM2		: MATRIX OR FOURTH ORDER TENSOR (NDIM2,NDIM3)
C-------------
C  OUTPUT :
C-------------
C  VM3		: MATRIX OR FOURTH ORDER TENSOR (NDIM1,NDIM3)
C-----------------------------------------------------------------------
C=======================================================================
C.1-----  Implicit, External
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C.2-----  Parameter
C.3-----  Dimension
      DIMENSION VM1(NTENS,NTENS),VM2(NTENS,NTENS),VM3(NTENS,NTENS)
C.4-----  Real,Integer,Complex,Double precision,Logical,Character
C.5-----  Common
C.6-----  Equivalence
C.7-----  Data
      DATA ZERO/0.D0/
C.8-----  Function Definition
C=======================================================================
      DO I=1,NTENS
         DO J=1,NTENS
            R=ZERO
            DO K=1,NTENS
               R=R+VM1(I,K)*VM2(K,J)
            END DO
         VM3(I,J)=R
         END DO
      END DO
C=======================================================================
      RETURN
      END
C
      SUBROUTINE AFFECT(N,V1,V2)
C=====================================================================+
C---------------------------------------------------------------------+
C------------  EXECUTION OF VECTORIAL OPERATION: V2 = V1  ------------+
C---------------------------------------------------------------------+
C=====================================================================+
C Input
C -------
C  N  :  Vector dimension
C  V1 :  Vector to copy
C
C Output
C -------
C  V2 :  Copied Vector
C=====================================================================+
C.1-----  Implicit, External
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C.2-----  Parameter
C.3-----  Dimension
      DIMENSION V1(N),V2(N)
C.4-----  Real,Integer,Complex,Double precision,Logical,Character
C.5-----  Common
C.6-----  Equivalence
C.7-----  Save,Data
C.8-----  Definition de fonctions
C=====================================================================+
      DO I=1,N
         V2(I)=V1(I)
      END DO
C=====================================================================+
      RETURN
      END
C
      SUBROUTINE TRANSPOSE(N,A,B)
C=====================================================================+
C---------------------------------------------------------------------+
C---------  EXECUTION OF MATRICIAL OPERATION: B = TRANSPOSE(A)  ------+
C---------------------------------------------------------------------+
C=====================================================================+
C Input
C -------
C  N -------> Dimension of matrices
C  A -------> Matrice to transpose
C
C Output
C -------
C  B -------> Transposed Matrice
C=====================================================================+
C.1-----  Implicit, External
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C.2-----  Parameter
C.3-----  Dimension
      DIMENSION A(N,N),B(N,N)
C.4-----  Real,Integer,Complex,Double precision,Logical,Character
C.5-----  Common
C.6-----  Equivalence
C.7-----  Save,Data
C.8-----  Function Definition
C======================================================================+
      DO I=1,N
         DO J=1,N
            B(J,I)=A(I,J)
         END DO
      END DO
C======================================================================+
      RETURN
      END
C
