
recursive Subroutine wateos_PH(p, h, T, dw, hx, energyscale, tolerance, ierr)
   use IAPWS97, only : cowat

  implicit none

  real*8 p,h, energyscale
  real*8 T,dw, hx
  real*8 tolerance

  integer ierr

  real*8 tx0, tx, dt, txd
  real*8 hw, hxd, dwmol
  logical succ
  integer itr

  tx0 = T ! get initial guess

  if (tx0 <1D0) then ! obtain initial guess
    tx0 = 100D0 ! or use constant cp
  endif

  itr= 0
  tx=tx0
  dt=-1D-7

  do
   succ = cowat(tx, p, dw, hw)
   hx= hw * energyscale
!   if(dabs((hx-h)/h)<= 1D-14) exit
    if(dabs((hx-h))<= tolerance) exit
   txd = tx + dt
   succ = succ .and. cowat(txd, p, dw, hw)
   hxd= hw * energyscale
   if ( .not. succ ) then
      print *, 'Wateos_PH: Error calling cowat. STOP'
      print *, p, h, t, tx, txd, hx, hxd
      stop
   endif
   tx = tx + ( h - hx)*dt / (hxd-hx)
!   print *, itr, tx, h,hx, hxd
   itr =itr+1
   if(itr>150)then
      print *,'Wateos_PH: Newton iteration does not converge within 150 times. STOP'
      print *, p, h, t, tx, hx-h
      stop
    endif
   enddo
  t=tx
  End Subroutine wateos_PH


!*************************************************************

 Subroutine steameos_PH(p, h, T, dg, hx, energyscale, tolerance, ierr)
  use IAPWS97, only : supst
  implicit none

  real*8 p,h, energyscale
  real*8 T, dg, hx
  real*8 tolerance

  integer ierr

  real*8 tx0, tx, dt,txd
  real*8 hg, hxd
  integer itr
  logical succ

  tx0 = T ! get initial guess

  if (tx0 <1D0) then ! obtain initial guess
    tx0 = 100D0 ! or use constant cp
  endif

  itr= 0
  tx=tx0
  dt=1D-7

  do
  succ=supst(tx, p, dg, hg)
   hx = hg * energyscale
!   if(dabs((hx-h)/h)<= 1D-14) exit
   if(dabs((hx-h))<= tolerance) exit
   txd = tx + dt
   succ=supst(txd, p, dg, hg)
   hxd=hg * energyscale
   tx = tx + ( h - hx)*dt / (hxd-hx)
   itr =itr+1
   if(itr>150)then
      print *,'steameos_PH: Newton iteration does not converge within 150 times. STOP'
      print *, p, h, t, tx, hx-h
      stop
    endif
   enddo
   t=tx
  End Subroutine steameos_PH

!*************************************************************

recursive subroutine water_steam_prop_PH(p, h, T, Sw, &
    Den,Denw, Dens, &
    hw, hs, dDendh, dDendp,  dhwdh,dhsdh,&
    dTdh, dswdh, ierror, dhwdp, dhsdp,dTdp)
   use IAPWS97, only : cowat, supst, tsat
  implicit none

  real*8, parameter::  fmwh2o = 18.01534d0
  real*8, parameter::  ps350c = 16.529D6, hw350c=1670.9D0
  real*8, parameter::  energyscale=1D-6 ! MJ/Kg
  real*8, parameter::  epi_p = 0.1D-0, epi_h = 0.1D0 * energyscale


  real*8, intent(inout)  :: p, h ! Pscal, KJ/Kg, C (initial guess)
  real*8, intent(out) :: T, Sw, hw,hs,Den,Denw,Dens,  dTdh, dDendp,dDendh, dhwdh,dhsdh, dswdh, dhwdp,dhsdp
  integer,intent(out) :: ierror  ! ierr = 10  critical point

  real*8 Ts
  real*8 dw, dwmol
  real*8 dg, dgmol, dgp,dgt,hg,hgp,hgt
  integer ierr
  integer :: iphase
  real*8 delp, delh
  real*8 dw0, dw1, hw0,hw1, sw0, sw1
  real*8 dg0, dg1, hg0,hg1, h1
  real*8 t0,t1, d0,d1
  real*8 dTdp
  logical succ

  real*8 arg1, arg2 !

  arg1 = 1.0D-14
  arg2 = 1.0D-14

  call  water_steam_prop_PH_ex(p, h, T, Sw, &
    Den,Denw, Dens, &
    hw, hs, dDendh, dDendp,  dhwdh,dhsdh,&
    dTdh, dswdh, ierror, dhwdp, dhsdp,dTdp, arg1, arg2)



end subroutine water_steam_prop_PH


!*************************************************************

!*************************************************************
! Subroutine water_steam_prop_PH_ex
!-------------------------------------------------------------
recursive subroutine water_steam_prop_PH_ex(p, h, T, Sw, &
    Den,Denw, Dens, &
    hw, hs, dDendh, dDendp,  dhwdh,dhsdh,&
    dTdh, dswdh, ierror, dhwdp, dhsdp,dTdp, arg1, arg2)
   use IAPWS97, only : cowat, supst, tsat
  implicit none

  real*8, parameter::  fmwh2o = 18.01534d0
  real*8, parameter::  ps350c = 16.529D6, hw350c=1670.9D0
  real*8, parameter::  energyscale=1D-6 ! MJ/Kg
  real*8, parameter::  epi_p = 0.1D-0, epi_h = 0.1D0 * energyscale


  real*8, intent(inout)  :: p, h ! Pscal, KJ/Kg, C (initial guess)
  real*8, intent(out) :: T, Sw, hw,hs,Den,Denw,Dens,  dTdh, dDendp,dDendh, dhwdh,dhsdh, dswdh, dhwdp,dhsdp
  integer,intent(out) :: ierror  ! ierr = 10  critical point
  real*8, intent(in)  :: arg1, arg2 ! optional input arguments to control convergence criteria

  real*8 Ts
  real*8 dw, dwmol
  real*8 dg, dgmol, dgp,dgt,hg,hgp,hgt
  integer ierr
  integer :: iphase
  real*8 delp, delh
  real*8 dw0, dw1, hw0,hw1, sw0, sw1
  real*8 dg0, dg1, hg0,hg1, h1
  real*8 t0,t1, d0,d1
  real*8 dTdp
  logical succ

  iphase = 0

! determine phase condition
  succ= TSAT( p, Ts)
! if(h<1D-1) h=1D-1
 if(p> ps350c )then
   if(h<=hw350c)iphase=1
   if(Ts>370D0) iphase=2
  else
    succ= cowat(Ts,p, dw, hw)
    hw=hw * energyscale
    succ= supst(Ts, p, dg,hg)
    hg=hg* energyscale

    if(h >= hg)then
      iphase = 2
    else if(h <= hw)then
      iphase=1
    else
      iphase=3
    endif
  endif


  select case(iphase)
  case(1) ! water only
    T= Ts-10D0
    call wateos_PH(p, h, T, dw, hw, energyscale, arg1, ierr)
    Sw=1D0
    dg=0D0
    hg=0D0
    hw=h !w/fmwh2o
    delp=epi_p
    delh=-epi_h
    Den=dw
    Denw=dw
    Dens=-1D0
    hs=0D0
  case(2)
    T= Ts+10D0
    call steameos_PH(p, h, T, dg, hg, energyscale, arg2, ierr)
    Sw=0D0
    dw=0D0
    hw=0D0
    hg=h!  g/fmwh2o
    delp=-epi_p
    delh=epi_h
    Den=Dg
    Denw=-1D0
    Dens=Den
    hs=hg
  case(3)
    T=Ts
    sw = 1D0/(1D0 -dw/dg*(hw-h)/(hg-h))
    Den= sw*dw+(1D0-sw)*dg
    Denw=dw
    Dens=dg
    hs=hg
    delp = epi_p
    delh = epi_h
    if((h+delh)>hg) delh=(hg-h)*0.5
  !  print *, 'eos: ',p,h,hs,hw, sw
   end select
   ierror=ierr
   if(h<0 .or.h> 3D3) print *,'h ph: ', p,t,h,hw,hg,iphase

 ! print *,'eos-den=: ', den
  select case (iphase)
  case(1)
   t0= T
   succ =  cowat( T, p, dw0, hw0)
   hw0=hw0* energyscale
   call  wateos_PH(p+delp, h, T, dw1, hw1, energyscale, arg1, ierr)
   t1=T
   dDendp=(dw1-dw0)/delp
   dTdp=(t1-t0)/delp
!+++++++++++++++++++++++++++++++

   dhwdp=(hw1-hw0)/delp
   dhsdp=0D0

!++++++++++++++++++++++++++++++
   call  wateos_PH(p, h+delh, T, dw1, hw1, energyscale, arg1, ierr)
   t1=T

   dDendh=(dw1-dw0)/delh
   dTdh=(t1-t0)/delh
   dhwdh=1D0
   dhsdh=0D0
   dswdh=0D0


  case(2)
   t0=T
   succ=supst(T, p,  dg0, hg0)
   hg0 =hg0 * energyscale
   call steameos_PH(p+delp, h, T, dg1, hg1, energyscale, arg2, ierr)
   t1=T
   dDendp=(dg1-dg0)/delp
   dTdp=(t1-t0)/delp
!++++++++++++++++++++++++++++

dhwdp=0D0
dhsdp=(hg1-hg0)/delp

!++++++++++++++++++++++++++++
   call  steameos_PH(p, h+delh, T, dw1, hg1, energyscale, arg2, ierr)
   t1=T
   dDendh=(dg1-dg0)/delh
   dTdh=(t1-t0)/delh
   dhwdh=0D0
   dhsdh=1D0
   dswdh=0D0



  case(3)
   t0=Ts
   dw0=dw; hw0=hw; sw0=sw
   dg0=dg; hg0=hg
   d0= Den
   succ = TSAT( p+delp, t1)
   dTdp=(t1-t0)/delp
   succ=cowat(t1,p+delp, dw, hw)
   succ=supst(t1, p+delp, dg,hg)
   hw=hw*energyscale; hg=hg*energyscale
   sw1 = 1D0/(1D0 -dw/dg*(hw-h)/(hg-h))
   d1=sw1*dw+(1D0-sw1)*dg
   dDendp=(d1-d0)/delp

!+++++++++++++++++++++++++++

   dhwdp=(hw-hw0)/delp
   dhsdp=(hg-hg0)/delp

!+++++++++++++++++++++++++++
   dTdh=0D0
   h1= h +delh
   sw1 = 1D0/(1D0 -dw0/dg0*(hw0-h1)/(hg0-h1))
   d1=sw1*dw0+(1D0-sw1)*dg0
   dDendh=(d1-d0)/delh
   sw=sw0
   dw=dw0
   hw=hw0
   dhwdh=0D0
   dhsdh=0D0
   dswdh=(sw1-sw0)/delh

  end select
  T=t0

 end subroutine water_steam_prop_PH_ex

!*************************************************************

!*************************************************************
! Subroutine water_steam_prop_PH
!-------------------------------------------------------------
recursive subroutine water_steam_prop_PH_noderiv(p, h, T, Sw, &
    Den,Denw, Dens, hw, hs,visw,viss,ierror)
   use IAPWS97, only : cowat, supst, tsat, visc
  implicit none

  real*8, parameter::  fmwh2o = 18.01534d0
  real*8, parameter::  epi_p = 1D0, epi_h=1D-3
  real*8, parameter::  ps350c = 16.529D6, hw350c=1670.9D0
  real*8, parameter::  energyscale=1D-6 ! MJ/Kg
  real*8, parameter::  arg1 = 1.0D-14, arg2=1.0D-14

  real*8, intent(in)  :: p, h ! Pscal, KJ/Kg, C (initial guess)
  real*8, intent(out) :: T, Sw, hw,hs,Den,Denw,Dens, viss,visw
  integer,intent(out) :: ierror  ! ierr = 10  critical point

  real*8 Ts
  real*8 dw
  real*8 dg, hg
  integer ierr
  integer :: iphase
  real*8 delp, delh
  real*8 dw0, dw1, hw0,hw1, sw0, sw1
  real*8 dg0, dg1, hg0,hg1, h1
  real*8 t0,t1, d0,d1
  real*8 dTdp
  logical succ

  iphase = 0

! determine phase condition
  ierror=0
  succ= TSAT( p, Ts)

 if(p> ps350c )then
   if(h<=hw350c)iphase=1
   if(Ts>370) iphase=2
  else
    succ= cowat(Ts,p, dw, hw)
    hw=hw * energyscale
    succ= supst(Ts, p, dg,hg)
    hg=hg* energyscale

    if(h >= hg)then
      iphase = 2
    else if(h <= hw)then
      iphase=1
    else
      iphase=3
    endif
  endif


  select case(iphase)
  case(1) ! water only
    T= Ts-10D0
    call wateos_PH(p, h, T, dw, hw, energyscale, arg1, ierr)
    Sw=1D0
    dg=0D0
    hg=0D0
    hw=h !w/fmwh2o
    delp=epi_p
    delh=-epi_h
    Den=dw
    Denw=dw
    Dens=-1D0
    hs=0D0
    ierror =ierr
    call viss_noderiv1(Denw,T, visw)
    viss=1D-6
  case(2)
    T= Ts+10D0
    call steameos_PH(p, h, T, dg, hg, energyscale, arg2, ierr)
    Sw=0D0
    dw=0D0
    hw=0D0
    hg=h!  g/fmwh2o
    delp=-epi_p
    delh=epi_h
    Den=Dg
    Denw=-1D0
    Dens=Den
    hs=hg
    call viss_noderiv1(Dens,T, viss)
    visw=5D-4
    ierror =ierr
  case(3)
    T=Ts
    sw = 1D0/(1D0 -dw/dg*(hw-h)/(hg-h))
    Den= sw*dw+(1D0-sw)*dg
    Denw=dw
    Dens=dg
    hs=hg
    delp = epi_p
    delh = epi_h
    call viss_noderiv1(Dens,T, viss)
    call viss_noderiv1(Denw,T, visw)
  !  print *, 'eos: ',p,h,hs,hw, sw
   end select
   if(h<0 .or.h> 3D3) print *,'h ph: ', p,t,h,hw,hg,iphase



 end subroutine water_steam_prop_PH_noderiv
