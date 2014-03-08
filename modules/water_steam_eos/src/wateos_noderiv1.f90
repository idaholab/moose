 SUBROUTINE wateos_noderiv1 (t,p,dw)

    implicit none

    real*8, intent(in) :: t   ! Temperature in centigrade.
    real*8, intent(in) :: p   ! Pressure in Pascals.
    real*8, intent(out) :: dw
    real*8 :: dwmol,hw
    integer :: ierr

    integer :: i

    real*8, save :: aa(0:22)
    real*8, save :: a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12

    real*8 :: beta,beta2x,beta4,theta,utheta,theta2x,theta18,theta20
    real*8 :: xx,yy,zz
    real*8 :: u0,u1,u2,u3,u4,u5,u6,u7,u8
!   real*8 :: u9
    real*8 :: v0,v1,v2,v3,v4,v20,v40
    real*8 :: term1,term2,term3,term4,term4p,term5,term6,term7
!   real*8 :: term2t,term3t,term3p,term4t,term5t,term5p,term6t,term6p,term7t,term7p
!   real*8 :: dv2t,dv2p,dv3t
    real*8 :: vr,ypt
!   real*8 :: yptt,zpt,zpp,vrpt,vrpp,cnv
    real*8 :: tc1,pc1,vc1,utc1,upc1,vc1mol,vc1molh
    real*8 :: zero,one,two,three,four,five,six,seven,eight,fnine,ten
    real*8 :: scale

!   save aa,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12

    data aa/ &
!-----data aa0,aa1,aa2,aa3/
         6.824687741d03,-5.422063673d02,-2.096666205d04, 3.941286787d04, &
!-----data aa4,aa5,aa6,aa7/
         -6.733277739d04, 9.902381028d04,-1.093911774d05, 8.590841667d04, &
!-----data aa8,aa9,aa10,aa11/
         -4.511168742d04, 1.418138926d04,-2.017271113d03, 7.982692717d00, &
!-----data aa12,aa13,aa14,aa15/
         -2.616571843d-2, 1.522411790d-3, 2.284279054d-2, 2.421647003d02, &
!-----data aa16,aa17,aa18,aa19/
         1.269716088d-10,2.074838328d-7, 2.174020350d-8, 1.105710498d-9, &
!-----data aa20,aa21,aa22/
         1.293441934d01, 1.308119072d-5, 6.047626338d-14/

    data a1,a2,a3,a4/ &
    8.438375405d-1, 5.362162162d-4, 1.720000000d00, 7.342278489d-2/
    data a5,a6,a7,a8/ &
    4.975858870d-2, 6.537154300d-1, 1.150000000d-6, 1.510800000d-5/
    data a9,a10,a11,a12/ &
    1.418800000d-1, 7.002753165d00, 2.995284926d-4, 2.040000000d-1/


    zero = 0.d0
    one = 1.d0
    two = 2.d0
    three = 3.d0
    four = 4.d0
    five = 5.d0
    six  = 6.d0
    seven = 7.d0
    eight = 8.d0
    fnine = 9.d0
    ten = 10.d0

    tc1 = 647.3d0
    pc1 = 2.212d7
    vc1 = 0.00317d0
    utc1 = one/tc1
    upc1 = one/pc1
    vc1mol = vc1*18.01534d0

    theta = (t+273.15d0)*utc1
    theta2x = theta*theta
    theta18 = theta**18.
    theta20 = theta18*theta2x

    beta = p*upc1
    beta2x = beta*beta
    beta4  = beta2x*beta2x

    yy = one-a1*theta2x-a2*theta**(-6.)
    xx = a3*yy*yy-two*(a4*theta-a5*beta)

!   Note: xx may become negative near the critical point-pcl.
    if (xx.gt.zero) then
      xx = sqrt(xx)
    else
      write(*,*) 'Warning: negative term in density (no deriv): ',t,p,xx
      xx = 1.e-6               !set arbitrarily
    end if
    zz = yy + xx
    u0 = -five/17.d0
    u1 = aa(11)*a5*zz**u0
    u2 = one/(a8+theta**11.)
    u3 = aa(17)+(two*aa(18)+three*aa(19)*beta)*beta
    u4 = one/(a7+theta18*theta)
    u5 = (a10+beta)**(-4.)
    u6 = a11-three*u5
    u7 = aa(20)*theta18*(a9+theta2x)
    u8 = aa(15)*(a6-theta)**9.

    vr = u1+aa(12)+theta*(aa(13)+aa(14)*theta)+u8*(a6-theta) &
         +aa(16)*u4-u2*u3-u6*u7+(three*aa(21)*(a12-theta) &
         +four*aa(22)*beta/theta20)*beta2x

    dwmol = one/(vr*vc1mol)
    dw = one/(vr*vc1)

!---calculate derivatives for water density
    ypt = six*a2*theta**(-7.)-two*a1*theta
!   zpt = ypt+(a3*yy*ypt-a4)/xx
!   zpp = a5/xx
!   u9 = u0*u1/zz

!   vrpt = u9*zpt+aa(13)+two*aa(14)*theta-ten*u8 &
!       -19.d0*aa(16)*u4*u4*theta18+11.d0*u2*u2*u3*theta**10 &
!       -aa(20)*u6*(18.d0*a9*theta18+20.d0*theta20)/theta &
!       -(three*aa(21)+80.d0*aa(22)*beta/(theta20*theta))*beta2x

!   vrpp = u9*zpp-u2*(two*aa(18)+six*aa(19)*beta)-12.d0*u7*u5/ &
!       (a10+beta)+(six*aa(21)*(a12-theta)+12.d0*aa(22)*beta/ &
!       theta20)*beta

!   cnv = -one/(vc1mol*vr*vr)
!   dwt = cnv*vrpt*utc1
!   dwp = cnv*vrpp*upc1

!   print *,'water_eos: ',p,t,dwp,cnv,vrpp,upc1

!---compute enthalpy internal energy and derivatives for water
    utheta = one/theta
    term1 = aa(0)*theta

    term2 = -aa(1)
!   term2t = zero
    do i = 3,10
      v1 = dfloat(i-2)*aa(i)*theta**(1.0*(i-1))
!     term2t = term2t+v1*utheta*dfloat(i-1)
      term2 = term2+v1
    end do

!   print *,'wateos-no: ',term2,term2t,v1

    v0 = u1/a5
    v2 = 17.d0*(zz/29.d0-yy/12.d0)+five*theta*ypt/12.d0
    v3 = a4*theta-(a3-one)*theta*yy*ypt
    v1 = zz*v2+v3
    term3 = v0*v1

!   yptt = -two*a1-42.d0*a2/theta**8
!   dv2t = 17.d0*(zpt/29.d0-ypt/12.d0)+five/12.d0*(ypt+theta*yptt)
!   dv3t = a4-(a3-one)*(theta*yy*yptt+yy*ypt+theta*ypt*ypt)
!   dv2p = 17.d0*zpp/29.d0
!   v4 = five*v1/(17.d0*zz)

!   term3t = v0*(zz*dv2t+(v2-v4)*zpt+dv3t)
!   term3p = v0*(zz*dv2p+(v2-v4)*zpp)

    v1 = fnine*theta+a6
    v20 = (a6-theta)
    v2 = v20**9.
    v3 = a7+20.d0*theta**19.
    v40 = a7+theta**19.
    v4 = one/(v40*v40)
    term4p = aa(12)-aa(14)*theta2x+aa(15)*v1*v2+aa(16)*v3*v4
    term4 = term4p*beta
!   term4t =(-two*aa(14)*theta+fnine*aa(15)*(v2-v1*v2/v20) &
!           +38.d0*theta18*aa(16)*(ten*v4-v3*v4/v40))*beta

    v1 = beta*(aa(17)+aa(18)*beta+aa(19)*beta2x)
    v2 = 12.d0*theta**11.+a8
    v4 = one/(a8+theta**11.)
    v3 = v4*v4
    term5 = v1*v2*v3
!   term5p = v3*v2*(aa(17)+two*aa(18)*beta+three*aa(19)*beta2x)
!   term5t = v1*(132.d0*v3*theta**10-22.d0*v2*v3*v4*theta**10)

    v1 = (a10+beta)**(-3.)+a11*beta
    v3 = (17.d0*a9+19.d0*theta2x)
    v2 = aa(20)*theta18*v3
    term6 = v1*v2
!   term6p = v2*(a11-three*(a10+beta)**(-4))
!   term6t = v1*aa(20)*theta18*(18.d0*v3*utheta+38.d0*theta)

    v1 = 21.d0*aa(22)/theta20*beta4
    v2 = aa(21)*a12*beta2x*beta
    term7 = v1+v2
!   term7p = beta2x*(three*aa(21)*a12+84.d0*aa(22)*beta/theta20)
!   term7t = -420.d0*aa(22)*beta4/(theta20*theta)

    vc1molh = vc1mol*scale

    v1 = pc1*vc1molh
    hw = (term1-term2+term3+term4-term5+term6+term7)*v1

!   hwp = (term3p+term4p-term5p+term6p+term7p)*vc1molh
!   hwt = (aa(0)-term2t+term3t+term4t-term5t+term6t+term7t)*v1*utc1

!   print *,'wateos-no: ',hw,term1,term2,term3,term4,term6,term7,term2t,v1

    ierr = 0

  end subroutine wateos_noderiv1
