MODULE MATRIX_OPERATIONS
!C
CONTAINS
!C
    SUBROUTINE MATRIX_INVERSE(matrix, det_matrix, inverse, n, errorflag)
!C
        USE CONSTANTS
        IMPLICIT NONE
!C
        INTEGER, INTENT(IN) :: n
        INTEGER, INTENT(OUT) :: errorflag
        REAL(R64), INTENT(IN) :: matrix(n,n)
        REAL(R64), INTENT(OUT) :: inverse(n,n)
        REAL(R64), INTENT(OUT) :: det_matrix
!C
        LOGICAL :: FLAG = .TRUE.
        INTEGER :: i, j, k
        REAL(R64) :: m
        REAL(R64) :: augmatrix(n,2*n) !augmented matrix
!C
        !Augment input matrix with an identity matrix
        DO i = 1, n
          DO j = 1, 2*n
            IF (j <= n ) THEN
              augmatrix(i,j) = matrix(i,j)
            ELSE IF ((i+n) == j) THEN
              augmatrix(i,j) = 1
            ELSE
              augmatrix(i,j) = 0
            ENDIF
          END DO
        END DO
        
        !Reduce augmented matrix to upper traingular form
        DO k =1, n-1
          IF (augmatrix(k,k) == 0) THEN
            FLAG = .FALSE.
            DO i = k+1, n
              IF (augmatrix(i,k) /= 0) THEN
                DO j = 1,2*n
                  augmatrix(k,j) = augmatrix(k,j)+augmatrix(i,j)
                END DO
                FLAG = .TRUE.
                EXIT
              ENDIF
              IF (FLAG .EQV. .FALSE.) THEN
                PRINT*, "Matrix is non - invertible"
                inverse = 0
                errorflag = -1
                return
              ENDIF
            END DO
          ENDIF
          DO j = k+1, n      
            m = augmatrix(j,k)/augmatrix(k,k)
            DO i = k, 2*n
              augmatrix(j,i) = augmatrix(j,i) - m*augmatrix(k,i)
            END DO
          END DO
        END DO
        
        !Test for invertibility
        det_matrix = ONE
        DO i = 1, n
          IF (augmatrix(i,i) == 0) THEN
            PRINT*, "Matrix is non - invertible"
            inverse = 0
            errorflag = -1
            return
          ENDIF
          det_matrix = det_matrix*augmatrix(i,i)
        END DO
        
        !Make diagonal elements as 1
        DO i = 1 , n
          m = augmatrix(i,i)
          DO j = i , (2 * n)        
               augmatrix(i,j) = (augmatrix(i,j) / m)
          END DO
        END DO
        
        !Reduced right side half of augmented matrix to identity matrix
        DO k = n-1, 1, -1
          DO i =1, k
          m = augmatrix(i,k+1)
            DO j = k, (2*n)
              augmatrix(i,j) = augmatrix(i,j) -augmatrix(k+1,j) * m
            END DO
          END DO
        END DO        
        
        !store answer
        DO i =1, n
          DO j = 1, n
            inverse(i,j) = augmatrix(i,j+n)
          END DO
        END DO
        errorflag = 0
!C
    END SUBROUTINE MATRIX_INVERSE
END MODULE MATRIX_OPERATIONS
