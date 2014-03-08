
 subroutine viss_noderiv1(rho,t, visc)

  ! Calculates dynamic viscosity of water or steam, given the density
  ! rho and temperature t, using the IAPWS industrial formulation 2008.
  ! Critical enhancement of viscosity near the critical point is not
  ! included.

  implicit none

  real*8 , intent(in):: rho,t
  real*8 , intent(out):: visc

  real*8, parameter :: tc_k = 273.15d0
  real*8, parameter :: dcritical=322.0d0
  real*8, parameter :: tcriticalk=647.096d0
  real*8, parameter :: mustar=1.00d-6
  integer,parameter :: ivs(21)=(/0,1,2,3,0,1,2,3,5,0,1,2,3,4,0,1,0,3,4,3,5/)
  integer,parameter :: jvs(21)=(/0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,4,4,5,6,6/)
  real*8, parameter :: h0v(0:3)= &
       (/1.67752d0, 2.20462d0, 0.6366564d0, -0.241605d0/)


  real*8, parameter :: h1v(21)=&
       (/5.20094d-1, 8.50895d-2, -1.08374d0, -2.89555d-1, 2.22531d-1,&
       9.99115d-1, 1.88797d0, 1.26613d0, 1.20573d-1, -2.81378d-1,&
       -9.06851d-1, -7.72479d-1, -4.89837d-1, -2.57040d-1, 1.61913d-1,&
       2.57399d-1, -3.25372d-2, 6.98452d-2, 8.72102d-3, -4.35673d-3,&
       -5.93264d-4/)


 ! Local variables:

  real*8 :: del,tk,tau
  real*8 :: tauipow(0:3),tspow(0:5),dspow(0:6)
  real*8 :: mu0,mu1,s0,s1
  integer i

  tk=t+tc_k
  tau=tk/tcriticalk
  del=rho/dcritical

  ! Calculate required inverse powers of tau:
  tauipow(0)=1.d0
  tauipow(1)=1.d0/tau
  tauipow(2)=tauipow(1)*tauipow(1)
  tauipow(3)=tauipow(2)*tauipow(1)

  ! Calculate required powers of shifted tau:
  tspow(0)=1.d0
  tspow(1)=tauipow(1)-1.d0
  tspow(2)=tspow(1)*tspow(1)
  tspow(3)=tspow(2)*tspow(1)
  tspow(4)=tspow(2)*tspow(2)
  tspow(5)=tspow(3)*tspow(2)

  ! Calculate required powers of shifted delta:
  dspow(0)=1.d0
  dspow(1)=del-1.d0
  dspow(2)=dspow(1)*dspow(1)
  dspow(3)=dspow(2)*dspow(1)
  dspow(4)=dspow(2)*dspow(2)
  dspow(5)=dspow(3)*dspow(2)
  dspow(6)=dspow(3)*dspow(3)

  ! Viscosity in dilute-gas limit:
  s0=0.d0
  do i=0,3
     s0=s0+h0v(i)*tauipow(i)
  end do
  mu0=100.d0*dsqrt(tau)/s0

  ! Contribution due to finite density:
  s1=0.d0
  do i=1,21
     s1=s1+tspow(ivs(i))*h1v(i)*dspow(jvs(i))
  end do
  mu1=dexp(del*s1)

  visc=mustar*mu0*mu1

  return
end subroutine viss_noderiv1

