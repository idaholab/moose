!--------------------------------------------------------------------------------
!
!  subroutine enthalpy_density_pt(H, D, P, T, ierr, nRegion)
!
!  This subroutine is a wrapper for the functions in module IAPWS97 to calculate
!  density and enthalpy as a function of temperature and pressure for water in a
!  region specified in IAPWS-IF97 standarad.
!
!  Input:
!  -----
!       P, pressure (Pa)
!       T, temperature (C)
!       nRegion,  = 1, for subcooled liquid
!                 = 2, for superheated vapor
!                 = 3, for region near critical point
!
!
!  Output:
!  ------
!       H, enthalpy  (J/kg)
!       D, density (kg/m^3)
!       nerr, error code
!             = 0 no error
!             = 1 error occured in calling module IAPWS subroutines
!             = 2 error in input region number
!
!
!  Dependency:
!        subroutines/functions in module IAPWS97, cowat, supst, and super
!
!  Note:
!
!--------------------------------------------------------------------------------

subroutine enthalpy_density_pt(H, D, P, T, ierr, nRegion)
use IAPWS97, only : cowat, supst, super

implicit none

double precision P

double precision T

double precision H

double precision D

integer nerr

integer nRegion

logical ierr

nerr    = 0

select case(nRegion)
case(1)
   ierr = cowat(T,P,D,H)
case(2)
   ierr = supst(T,P,D,H)
case(3)
   ierr = super(T,P,D,H)
case default
   nerr = 2
end select

if( nerr .eq. 0) then
 if(ierr) then
   nerr = 0
 else
   nerr = 1
 endif
endif

end subroutine enthalpy_density_pt
!--------------------------------------------------------------------------------
!
! subroutine boundary_23(P, T, N)
!
! This subroutine wraps the functions in the module IAPWS97 to calculate pressure
! as a funciton of tempeature or temperature as a function of pressure on the
! boundary curve that seperates region 2 and region 3 specified in IAPWS-IF97
!
! Input/Output:
!
!   P, pressure (Pa)
!   T, temperature (C)
!
! Input:
! -----
!   N,
!      = 1, T is input and P is output
!      otherwise, T is output and P is input
!
!
!  Dependency:
!        subroutines/functions in module IAPWS97: b23p and b23t
!
!--------------------------------------------------------------------------------

subroutine boundary_23(P, T, N)
use IAPWS97, only : b23p, b23t
double precision P
double precision T

integer N

if( N .eq. 1) then
     P = b23p(T)
else
     T = b23t(P)
endif

end subroutine boundary_23

!--------------------------------------------------------------------------------
!
! subroutine saturation(P, T, N, nerr)
!
! This subroutine wraps the functions in the module IAPWS97 to calculate pressure
! as a funciton of tempeature or temperature as a function of pressure on the
! saturation curve (region 4) specified in IAPWS-IF97
!
! Input/Output:
!
!   P, pressure (Pa)
!   T, temperature (C)
!
! Input:
! -----
!   N,
!      = 1, T is input and P is output
!      otherwise, T is output and P is input
!
! Output:
! ------
!   nerr, error code
!         = 0, no error
!         = 1, error occurred in calling functions in module IAPWS97
!
!  Dependency:
!        subroutines/functions in module IAPWS97: tsat, sat
!
!--------------------------------------------------------------------------------

subroutine saturation(P, T, N, nerr)
use IAPWS97, only : tsat, sat

double precision P
double precision T

integer N
integer nerr

logical ierr


if ( N .eq. 1) then
     ierr = sat(T,P)
else
     ierr = tsat(P,T)
endif

if(ierr) then
   nerr = 0
else
   nerr = 1
endif

end subroutine saturation

!--------------------------------------------------------------------------------
!
!  double precision function viscosity(rho, T)
!
!  The function wrapps the function in module IAPWS to calculate viscosity of water
!  as a function of density and temperature.
!
!   Input:
!   -----
!
!   rho, density in (kg/m^3)
!   T,   temperature in (C)
!
!
!   Output:
!   ------
!   viscosity, viscosity in (Pa-sec)
!
!  Dependency:
!        subroutines/functions in module IAPWS97: visc
!--------------------------------------------------------------------------------

double precision function viscosity(rho, T)
use IAPWS97, only : visc

double precision rho
double precision T

viscosity = visc(rho, T)

end function viscosity
