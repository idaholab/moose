MODULE SOLVE_SUBS
!C
CONTAINS
!C
    SUBROUTINE NEWTON_INC(x, a, b, n)
!C
        USE CONSTANTS
        IMPLICIT NONE
!C
        REAL(R64), INTENT(IN) :: a(n,n)
        REAL(R64), INTENT(IN) :: b(n)
!C
        INTEGER, INTENT(IN) :: N
!C
        REAL(R64), INTENT(OUT) :: x(n)
!C
        REAL(R64) :: a_tmp(n,n), d
        INTEGER :: indx(n)
!C
        a_tmp = a
        x = b
!C
        CALL LUDCMP(a_tmp,n,n,indx,d)
        CALL LUBKSB(a_tmp,n,n,indx,x)
!C
    END SUBROUTINE NEWTON_INC
!C
    SUBROUTINE LUDCMP(a,n,np,indx,d)
!C
        USE CONSTANTS
        IMPLICIT NONE
!C
        INTEGER, PARAMETER :: NMAX=500
!C
        REAL(R64), INTENT(INOUT) :: d
!C
        INTEGER, INTENT(INOUT) :: indx(n)
!C
        INTEGER, INTENT(IN) :: n, np
!C
        REAL(R64), INTENT(INOUT) :: a(np,np)
!C
        REAL(R64) :: aamax, dum, sum, vv(NMAX)
!C
        INTEGER :: i, j, imax, k
!C
        d=one
        do i=1,n
            do j=1,n
                if (abs(a(i,j)).gt.aamax) aamax=abs(a(i,j))
            end do
            vv(i)=1./aamax
        end do
        do j=1,n
            do i=1,j-1
                sum=a(i,j)
                do k=1,i-1
                    sum=sum-a(i,k)*a(k,j)
                enddo
                a(i,j)=sum
            enddo
            aamax=0.
            do i=j,n
                sum=a(i,j)
                do k=1,j-1
                    sum=sum-a(i,k)*a(k,j)
                enddo
                a(i,j)=sum
                dum=vv(i)*abs(sum)
                if (dum.ge.aamax) then
                    imax=i
                    aamax=dum
                endif
            enddo
            if (j.ne.imax) then
                do k=1,n
                    dum=a(imax,k)
                    a(imax,k)=a(j,k)
                    a(j,k)=dum
                enddo
                d=-d
                vv(imax)=vv(j)
            endif
            indx(j)=imax
            if(a(j,j).eq.0.) a(j,j)=tiny
            if(j.ne.n) then
                dum=1./a(j,j)
                do i=j+1,n
                    a(i,j)=a(i,j)*dum
                enddo
            endif
        enddo

!C
    END SUBROUTINE LUDCMP
!C
    SUBROUTINE LUBKSB(a,n,np,indx,b)
!C
        USE CONSTANTS
        IMPLICIT NONE
!C
        REAL(R64), INTENT(INOUT) :: b(n)
!C
        INTEGER, INTENT(IN) :: n, np, indx(n)
!C
        REAL(R64), INTENT(IN) :: a(np,np)
!C
        REAL(R64) :: sum
!C
        INTEGER :: i, ii, j, ll
!C
        ii=0
        do i=1,n
            ll=indx(i)
            sum=b(ll)
            b(ll)=b(i)
            if (ii.ne.0) then
                do j=ii,i-1
                    sum=sum-a(i,j)*b(j)
                enddo
            else if (sum.ne.zero) then
                ii=i
            end if
            b(i)=sum
        end do
        do i=n,1,-1
            sum=b(i)
            do j=i+1,n
                sum=sum-a(i,j)*b(j)
            end do
            b(i)=sum/a(i,i)
        end do
!C
    END SUBROUTINE LUBKSB
!C
END MODULE
