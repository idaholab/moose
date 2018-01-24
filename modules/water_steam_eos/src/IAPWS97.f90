module IAPWS97

  !  IAPWS-97 industrial thermodynamic formulation, as described by:

  ! Wagner, W., Cooper, J.R., Dittman, A., Kijima, J., Kretzschmar, H.-J., Kruse, A., Mares, R.,
  ! Oguchi, K., Sato, H., Stocker, I., Sifner, O., Takaishi, Y., Tanishita, I., Trubenbach, J. and
  ! Willkommen, Th., 1997.  The IAPWS Industrial Formulation 1997 for the Thermodynamic Properties of
  ! Water and Steam.  Trans. ASME 150(122), 150-182.

  ! Viscosity function visc() is described by:
  ! IAPWS, 2008.  Release on the IAPWS Formulation 2008 for the Viscosity of Ordinary Water Substance.

  implicit none

  ! General constants:
  double precision, parameter, public:: rconst=0.461526d3         ! Gas constant
  double precision, parameter, public:: tc_k=273.15d0             ! Conversion from Celsius to Kelvin
  double precision, parameter, public:: tcriticalk=647.096d0      ! Critical temperature (Kelvin)
  double precision, parameter, public:: tcritical=tcriticalk-tc_k
  double precision, parameter, public:: dcritical=322.0d0         ! Critical density (kg/m3)
  double precision, parameter, public:: pcritical=22.064d6        ! Critical pressure (Pa)

  ! -- Region 1 constants: --------------------------------------------------

  ! scaling values for pressure and temperature:
  double precision, parameter, private:: pstar1=16.53d6,tstar1=1386.d0

  double precision, parameter, private:: nr1(34)=&                         ! coefficients n
       (/0.14632971213167d0, -0.84548187169114d0, -0.37563603672040d1,&
       0.33855169168385d1, -0.95791963387872d0, 0.15772038513228d0,&
       -0.16616417199501d-1,0.81214629983568d-3, 0.28319080123804d-3,&
       -0.60706301565874d-3, -0.18990068218419d-1,-0.32529748770505d-1,&
       -0.21841717175414d-1, -0.52838357969930d-4, -0.47184321073267d-3,&
       -0.30001780793026d-3, 0.47661393906987d-4, -0.44141845330846d-5,&
       -0.72694996297594d-15,-0.31679644845054d-4, -0.28270797985312d-5,&
       -0.85205128120103d-9, -0.22425281908000d-5,-0.65171222895601d-6,&
       -0.14341729937924d-12, -0.40516996860117d-6, -0.12734301741641d-8,&
       -0.17424871230634d-9, -0.68762131295531d-18, 0.14478307828521d-19,&
       0.26335781662795d-22,-0.11947622640071d-22, 0.18228094581404d-23,&
       -0.93537087292458d-25/)

  integer, parameter, private:: ir1(34)=&             ! powers i
       (/0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,&
       2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 8, 8,&
       21, 23, 29, 30, 31, 32/)

  integer, parameter, private:: jr1(34)=&             ! powers j
       (/-2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1,&
       3, -3, 0, 1, 3, 17, -4, 0, 6, -5, -2, 10, -8,&
       -11, -6, -29, -31, -38, -39, -40, -41/)

  ! -- Region 2 constants: --------------------------------------------------

  ! scaling values for pressure and temperature:
  double precision,parameter, private :: pstar2=1.0d6,tstar2=540.d0

  double precision, parameter, private:: n0r2(9)=&                         ! coefficients n0
       (/-0.96927686500217d1,  0.10086655968018d2,-0.56087911283020d-2,&
       0.71452738081455d-1, -0.40710498223928d0, 0.14240819171444d1,&
       -0.43839511319450d1,  -0.28408632460772d0, 0.21268463753307d-1/)

  double precision, parameter, private:: nr2(43)=&                                                 ! coefficients n
       (/-0.17731742473213d-2, -0.17834862292358d-1,-0.45996013696365d-1, -0.57581259083432d-1, &
       -0.50325278727930d-1, -0.33032641670203d-4,-0.18948987516315d-3, -0.39392777243355d-2,&
       -0.43797295650573d-1, -0.26674547914087d-4, 0.20481737692309d-7,  0.43870667284435d-6,&
       -0.32277677238570d-4, -0.15033924542148d-2, -0.40668253562649d-1, -0.78847309559367d-9,&
       0.12790717852285d-7,  0.48225372718507d-6, 0.22922076337661d-5, -0.16714766451061d-10,&
       -0.21171472321355d-2, -0.23895741934104d2, -0.59059564324270d-17,-0.12621808899101d-5,&
       -0.38946842435739d-1,  0.11256211360459d-10, -0.82311340897998d1,   0.19809712802088d-7,&
       0.10406965210174d-18,-0.10234747095929d-12, -0.10018179379511d-8, -0.80882908646985d-10,&
       0.10693031879409d0,  -0.33662250574171d0, 0.89185845355421d-24, 0.30629316876232d-12,&
       -0.42002467698208d-5, -0.59056029685639d-25, 0.37826947613457d-5, -0.12768608934681d-14,&
       0.73087610595061d-28, 0.55414715350778d-16, -0.94369707241210d-6 /)

  integer, parameter, private:: j0r2(9)=(/0, 1, -5, -4, -3, -2, -1, 2, 3/)  ! powers j0

  integer, parameter, private:: ir2(43)=&                                   ! powers i
       (/1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,&
       4, 4, 4, 5, 6, 6, 6, 7, 7, 7, 8, 8, 9, 10, 10,&
       10, 16, 16, 18, 20, 20, 20, 21, 22, 23, 24, 24, 24/)

  integer, parameter, private:: jr2(43)=&                                   ! powers j
       (/0, 1, 2, 3, 6, 1, 2, 4, 7, 36, 0, 1, 3, 6,&
       35, 1, 2, 3, 7, 3, 16, 35, 0, 11, 25, 8, 36,&
       13, 4, 10, 14, 29, 50, 57, 20, 35, 48, 21, 53,&
       39, 26, 40, 58/)

  ! -- Region 3 constants: --------------------------------------------------

  ! scaling values for density and temperature:
  double precision, parameter, private:: dstar3=dcritical,tstar3=tcriticalk

  double precision, parameter, private:: nr3(40)=&                                            ! coefficients n
       (/0.10658070028513d1, -0.15732845290239d2,0.20944396974307d2,-0.76867707878716d1,&
       0.26185947787954d1, -0.28080781148620d1,0.12053369696517d1,-0.84566812812502d-2,&
       -0.12654315477714d1, -0.11524407806681d1, 0.88521043984318d0,-0.64207765181607d0,&
       0.38493460186671d0, -0.85214708824206d0, 0.48972281541877d1,-0.30502617256965d1,&
       0.39420536879154d-1, 0.12558408424308d0, -0.27999329698710d0,0.13899799569460d1,&
       -0.20189915023570d1, -0.82147637173963d-2, -0.47596035734923d0,0.43984074473500d-1,&
       -0.44476435428739d0, 0.90572070719733d0, 0.70522450087967d0,0.10770512626332d0,&
       -0.32913623258954d0, -0.50871062041158d0, -0.22175400873096d-1,0.94260751665092d-1,&
       0.16436278447961d0, -0.13503372241348d-1, -0.14834345352472d-1,0.57922953628084d-3,&
       0.32308904703711d-2, 0.80964802996215d-4, -0.16557679795037d-3,-0.44923899061815d-4/)

  integer, parameter, private:: ir3(40)=&                            ! powers i
       (/0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,&
       3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 8, 9,&
       9 , 10, 10, 11/)

  integer, parameter, private:: jr3(40)=&                            ! powers j
       (/0, 0, 1, 2, 7, 10, 12, 23, 2, 6, 15, 17, 0, 2, 6, 7, 22,&
       26, 0, 2, 4, 16, 26, 0, 2, 4, 26, 1, 3, 26, 0, 2, 26, 2,&
       26, 2, 26, 0, 1, 26/)

  ! -- Region 4 constants: --------------------------------------------------

  double precision, parameter, private:: pstar4=1.0d6  ! scaling value for pressure

  double precision, parameter, private:: nr4(10)=&                      ! coefficients n
       (/0.11670521452767d4, -0.72421316703206d6, -0.17073846940092d2,&
       0.12020824702470d5, -0.32325550322333d7, 0.14915108613530d2,&
       -0.48232657361591d4, 0.40511340542057d6, -0.23855557567849d0,&
       0.65017534844798d3/)

  !  -- Boundary between regions 2 & 3: --------------------------------------

  double precision, parameter, private:: nr23(5)=&
       (/0.34805185628969d3, -0.11671859879975d1, 0.10192970039326d-2,&
       0.57254459862746d3, 0.13918839778870d2/)

! -- Constants for dynamic viscosity calculation: -------------------------

  double precision, parameter, private:: mustar=1.00d-6

  double precision, parameter, private:: h0v(0:3)= &
       (/1.67752d0, 2.20462d0, 0.6366564d0, -0.241605d0/)

  double precision, parameter, private:: h1v(21)=&
       (/5.20094d-1, 8.50895d-2, -1.08374d0, -2.89555d-1, 2.22531d-1,&
       9.99115d-1, 1.88797d0, 1.26613d0, 1.20573d-1, -2.81378d-1,&
       -9.06851d-1, -7.72479d-1, -4.89837d-1, -2.57040d-1, 1.61913d-1,&
       2.57399d-1, -3.25372d-2, 6.98452d-2, 8.72102d-3, -4.35673d-3,&
       -5.93264d-4/)

  integer,parameter, private:: ivs(21)=(/0,1,2,3,0,1,2,3,5,0,1,2,3,4,0,1,0,3,4,3,5/)

  integer,parameter, private:: jvs(21)=(/0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,4,4,5,6,6/)

contains

!------------------------------------------------------------------------

recursive logical function cowat(t,p,d,h)

  ! Density d and internal energy u of liquid water as a function of
  ! temperature t (deg C) and pressure p (Pa).

  ! Returns false if called outside its operating range (t<=350 deg C, p<=100 MPa).

  implicit none
  double precision, intent(in):: t,p
  double precision, intent(out):: d,h

  ! Local variables:
  double precision:: tk,pi,tau,gampi,gamt,rt,u
  double precision:: pspow(-1:32),tspow(-42:17)
  integer:: i
  ! This tolerance allows cowat to be called just outside its
  ! nominal operating range when doing transitions to supercritical:
  !double precision, parameter:: ttol=1.0d-3
  double precision, parameter:: ttol=2.0d0

  ! Check input:
  if ((t<=350.0d0+ttol).and.(p<=100.d6)) then
     !
     tk=t+tc_k
     pi=p/pstar1
     tau=tstar1/tk

     ! Calculate required powers of shifted pi:
     pspow(0)=1.d0
     pspow(1)=7.1d0-pi
     pspow(2)=pspow(1)*pspow(1)
     pspow(3)=pspow(2)*pspow(1)
     pspow(4)=pspow(2)*pspow(2)
     pspow(5)=pspow(3)*pspow(2)
     pspow(7)=pspow(4)*pspow(3)
     pspow(8)=pspow(4)*pspow(4)
     pspow(20)=pspow(8)*pspow(8)*pspow(4)
     pspow(21)=pspow(20)*pspow(1)
     pspow(22)=pspow(20)*pspow(2)
     pspow(23)=pspow(20)*pspow(3)
     pspow(28)=pspow(23)*pspow(5)
     pspow(29)=pspow(22)*pspow(7)
     pspow(30)=pspow(22)*pspow(8)
     pspow(31)=pspow(23)*pspow(8)
     pspow(32)=pspow(28)*pspow(4)
     pspow(-1)=1.d0/pspow(1)

     ! Calculate required powers of shifted tau:
     tspow(0)=1.d0
     tspow(1)=tau-1.222d0
     tspow(2)=tspow(1)*tspow(1)
     tspow(3)=tspow(2)*tspow(1)
     tspow(4)=tspow(2)*tspow(2)
     tspow(5)=tspow(3)*tspow(2)
     tspow(6)=tspow(3)*tspow(3)
     tspow(9)=tspow(5)*tspow(4)
     tspow(10)=tspow(5)*tspow(5)
     tspow(16)=tspow(10)*tspow(6)
     tspow(17)=tspow(16)*tspow(1)
     tspow(-1)=1.d0/tspow(1)
     tspow(-2)=tspow(-1)*tspow(-1)
     tspow(-3)=tspow(-2)*tspow(-1)
     tspow(-4)=tspow(-2)*tspow(-2)
     tspow(-5)=tspow(-3)*tspow(-2)
     tspow(-6)=tspow(-3)*tspow(-3)
     tspow(-7)=tspow(-4)*tspow(-3)
     tspow(-8)=tspow(-4)*tspow(-4)
     tspow(-9)=tspow(-5)*tspow(-4)
     tspow(-10)=tspow(-5)*tspow(-5)
     tspow(-11)=tspow(-6)*tspow(-5)
     tspow(-12)=tspow(-6)*tspow(-6)
     tspow(-29)=tspow(-10)*tspow(-10)*tspow(-9)
     tspow(-30)=tspow(-29)*tspow(-1)
     tspow(-31)=tspow(-29)*tspow(-2)
     tspow(-32)=tspow(-29)*tspow(-3)
     tspow(-38)=tspow(-32)*tspow(-6)
     tspow(-39)=tspow(-32)*tspow(-7)
     tspow(-40)=tspow(-32)*tspow(-8)
     tspow(-41)=tspow(-32)*tspow(-9)
     tspow(-42)=tspow(-39)*tspow(-3)

     gampi=0.0d0
     gamt=0.0d0
     do i=1,34
        gampi=gampi-nr1(i)*ir1(i)*pspow(ir1(i)-1)*tspow(jr1(i))
        gamt=gamt+nr1(i)*pspow(ir1(i))*jr1(i)*tspow(jr1(i)-1)
     end do

     rt=rconst*tk

     d=pstar1/(rt*gampi)
     u=rt*(tau*gamt-pi*gampi)
     h=u+p/d
     cowat=.true.

  else

     cowat=.false.

  end if

  return
end function cowat

!------------------------------------------------------------------------

recursive logical function supst(t,p,d,h)

  ! Density d and internal energy u of dry steam as a function of
  ! temperature t (deg C) and pressure p (Pa)

  ! Returns false if called outside its operating range (t<=1000 deg C, p<=100 MPa).

  implicit none
  double precision, intent(in):: t,p
  double precision, intent(out):: d,h

  ! Local variables:
  double precision:: tk,pi,tau,gampi0,gampir,gamt0,gamtr,gampi,rt,u
  double precision:: taupow(-6:2),pipow(0:24),tspow(-1:58)
  integer:: i

  ! Check input:
  if ((t<=1000.0d0).and.(p<=100.d6)) then

     tk=t+tc_k
     pi=p/pstar2
     tau=tstar2/tk

     ! Calculate required powers of tau:
     taupow(0)=1.d0
     taupow(1)=tau
     taupow(2)=taupow(1)*taupow(1)
     taupow(-1)=1.d0/tau
     taupow(-2)=taupow(-1)*taupow(-1)
     taupow(-3)=taupow(-2)*taupow(-1)
     taupow(-4)=taupow(-2)*taupow(-2)
     taupow(-5)=taupow(-3)*taupow(-2)
     taupow(-6)=taupow(-3)*taupow(-3)

     ! Calculate required powers of pi:
     pipow(0)=1.d0
     pipow(1)=pi
     pipow(2)=pipow(1)*pipow(1)
     pipow(3)=pipow(2)*pipow(1)
     pipow(4)=pipow(2)*pipow(2)
     pipow(5)=pipow(3)*pipow(2)
     pipow(6)=pipow(3)*pipow(3)
     pipow(7)=pipow(4)*pipow(3)
     pipow(8)=pipow(4)*pipow(4)
     pipow(9)=pipow(5)*pipow(4)
     pipow(10)=pipow(5)*pipow(5)
     pipow(15)=pipow(8)*pipow(7)
     pipow(16)=pipow(8)*pipow(8)
     pipow(17)=pipow(9)*pipow(8)
     pipow(18)=pipow(9)*pipow(9)
     pipow(19)=pipow(10)*pipow(9)
     pipow(20)=pipow(10)*pipow(10)
     pipow(21)=pipow(15)*pipow(6)
     pipow(22)=pipow(15)*pipow(7)
     pipow(23)=pipow(15)*pipow(8)
     pipow(24)=pipow(15)*pipow(9)

     ! Calculate required powers of shifted tau:
     tspow(0)=1.d0
     tspow(1)=tau-0.5d0
     tspow(2)=tspow(1)*tspow(1)
     tspow(3)=tspow(2)*tspow(1)
     tspow(4)=tspow(2)*tspow(2)
     tspow(5)=tspow(3)*tspow(2)
     tspow(6)=tspow(3)*tspow(3)
     tspow(7)=tspow(4)*tspow(3)
     tspow(8)=tspow(4)*tspow(4)
     tspow(9)=tspow(5)*tspow(4)
     tspow(10)=tspow(5)*tspow(5)
     tspow(11)=tspow(6)*tspow(5)
     tspow(12)=tspow(6)*tspow(6)
     tspow(13)=tspow(7)*tspow(6)
     tspow(14)=tspow(7)*tspow(7)
     tspow(15)=tspow(8)*tspow(7)
     tspow(16)=tspow(8)*tspow(8)
     tspow(19)=tspow(10)*tspow(9)
     tspow(20)=tspow(10)*tspow(10)
     tspow(21)=tspow(11)*tspow(10)
     tspow(24)=tspow(12)*tspow(12)
     tspow(25)=tspow(13)*tspow(12)
     tspow(26)=tspow(13)*tspow(13)
     tspow(28)=tspow(14)*tspow(14)
     tspow(29)=tspow(15)*tspow(14)
     tspow(34)=tspow(19)*tspow(15)
     tspow(35)=tspow(19)*tspow(16)
     tspow(36)=tspow(20)*tspow(16)
     tspow(38)=tspow(19)*tspow(19)
     tspow(39)=tspow(20)*tspow(19)
     tspow(40)=tspow(20)*tspow(20)
     tspow(47)=tspow(26)*tspow(21)
     tspow(48)=tspow(24)*tspow(24)
     tspow(49)=tspow(25)*tspow(24)
     tspow(50)=tspow(25)*tspow(25)
     tspow(52)=tspow(26)*tspow(26)
     tspow(53)=tspow(28)*tspow(25)
     tspow(56)=tspow(28)*tspow(28)
     tspow(57)=tspow(29)*tspow(28)
     tspow(58)=tspow(29)*tspow(29)
     tspow(-1)=1.d0/tspow(1)

     gampi0=1.d0/pi

     gamt0=0.d0
     do i=1,9
        gamt0=gamt0+n0r2(i)*j0r2(i)*taupow(j0r2(i)-1)
     end do

     gampir=0.d0
     gamtr=0.d0
     do i=1,43
        gampir=gampir+nr2(i)*ir2(i)*pipow(ir2(i)-1)*tspow(jr2(i))
        gamtr=gamtr+nr2(i)*pipow(ir2(i))*jr2(i)*tspow(jr2(i)-1)
     end do

     gampi=gampi0+gampir
     rt=rconst*tk

     d=pstar2/(rt*gampi)
     u=rt*(tau*(gamt0+gamtr)-pi*gampi)
     h=u+p/d
     supst=.true.

  else

     supst=.false.

  end if

  return
end function supst

!------------------------------------------------------------------------

recursive logical function super(d,t,p,h)

  ! Pressure p and internal energy u of supercritical water/steam
  ! as a function of density d and temperature t (deg C).

  ! Returns false if resulting pressure is outside its operating range (p<=100 MPa).

  implicit none
  double precision, intent(in):: d,t
  double precision, intent(out):: p,h

  ! Local variables:
  double precision:: tk,delta,tau,u
  double precision:: taupow(-1:26),delpow(-1:11)
  double precision:: phidelta,phitau,rt
  integer:: i

  tk=t+tc_k
  tau=tstar3/tk
  delta=d/dstar3

  ! Calculate required powers of tau:
  taupow(0)=1.d0
  taupow(1)=tau
  taupow(2)=taupow(1)*taupow(1)
  taupow(3)=taupow(2)*taupow(1)
  taupow(4)=taupow(2)*taupow(2)
  taupow(5)=taupow(3)*taupow(2)
  taupow(6)=taupow(3)*taupow(3)
  taupow(7)=taupow(4)*taupow(3)
  taupow(9)=taupow(5)*taupow(4)
  taupow(10)=taupow(5)*taupow(5)
  taupow(11)=taupow(6)*taupow(5)
  taupow(12)=taupow(6)*taupow(6)
  taupow(14)=taupow(7)*taupow(7)
  taupow(15)=taupow(9)*taupow(6)
  taupow(16)=taupow(9)*taupow(7)
  taupow(17)=taupow(10)*taupow(7)
  taupow(21)=taupow(11)*taupow(10)
  taupow(22)=taupow(11)*taupow(11)
  taupow(23)=taupow(12)*taupow(11)
  taupow(25)=taupow(14)*taupow(11)
  taupow(26)=taupow(14)*taupow(12)
  taupow(-1)=1.d0/tau

  ! Calculate required powers of delta:
  delpow(0)=1.d0
  delpow(1)=delta
  delpow(2)=delpow(1)*delpow(1)
  delpow(3)=delpow(2)*delpow(1)
  delpow(4)=delpow(2)*delpow(2)
  delpow(5)=delpow(3)*delpow(2)
  delpow(6)=delpow(3)*delpow(3)
  delpow(7)=delpow(4)*delpow(3)
  delpow(8)=delpow(4)*delpow(4)
  delpow(9)=delpow(5)*delpow(4)
  delpow(10)=delpow(5)*delpow(5)
  delpow(11)=delpow(6)*delpow(5)
  delpow(-1)=1.d0/delta

  phidelta=nr3(1)*delpow(-1)
  do i=2,40
     phidelta=phidelta+nr3(i)*ir3(i)*delpow(ir3(i)-1)*taupow(jr3(i))
  end do

  phitau=0.d0
  do i=2,40
     phitau=phitau+nr3(i)*delpow(ir3(i))*jr3(i)*taupow(jr3(i)-1)
  end do

  rt=rconst*tk
  p=d*rt*delta*phidelta
  u=rt*tau*phitau
  h=u+p/d
  ! Check output:
  super=(p<=100.0d6)

  return
end function super

!------------------------------------------------------------------------

recursive logical function sat(t,p)

  ! Saturation pressure as a function of temperature.

  ! Returns false if called outside its operating range (0 <= t <= critical temperature).

  implicit none
  double precision, intent(in)::t
  double precision, intent(out):: p

  ! Local variables:
  double precision:: tk
  double precision:: theta,theta2,a,b,c,x

  if ((t>=0.d0).and.(t<=tcritical)) then
     tk=t+tc_k
     theta=tk+nr4(9)/(tk-nr4(10))
     theta2=theta*theta
     a=theta2+nr4(1)*theta+nr4(2)
     b=nr4(3)*theta2+nr4(4)*theta+nr4(5)
     c=nr4(6)*theta2+nr4(7)*theta+nr4(8)
     x=2.d0*c/(-b+dsqrt(b*b-4.d0*a*c))
     x=x*x
     p=pstar4*x*x
     sat=.true.
  else
     sat=.false.
  end if

  return
end function sat

!------------------------------------------------------------------------

recursive double precision function visc(rho,t)

  ! Calculates dynamic viscosity of water or steam, given the density
  ! rho and temperature t, using the IAPWS industrial formulation 2008.
  ! Critical enhancement of viscosity near the critical point is not
  ! included.

  implicit none

  double precision, intent(in):: rho,t

  ! Local variables:

  double precision:: del,tk,tau
  double precision:: tauipow(0:3),tspow(0:5),dspow(0:6)
  double precision:: mu0,mu1,s0,s1
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
end function visc

!------------------------------------------------------------------------

recursive logical function tsat(p,t)

  ! Saturation temperature (deg C) as a function of pressure.

  ! Returns false if called outside its operating range (611.213 Pa <= p <= critical pressure).

  implicit none
  double precision, intent(in):: p
  double precision, intent(out):: t

  ! Local variables:
  double precision:: beta,beta2,d,e,f,g,x

  if ((p>=611.213d0).and.(p<=pcritical)) then

     beta2=dsqrt(p/pstar4)
     beta=dsqrt(beta2)
     e=beta2+nr4(3)*beta+nr4(6)
     f=nr4(1)*beta2+nr4(4)*beta+nr4(7)
     g=nr4(2)*beta2+nr4(5)*beta+nr4(8)
     d=2.0d0*g/(-f-dsqrt(f*f-4.d0*e*g))
     x=nr4(10)+d
     t=0.5d0*(nr4(10)+d-dsqrt(x*x-4.d0*(nr4(9)+nr4(10)*d)))-tc_k
     tsat=.true.

  else
     tsat=.false.
  end if

  return
end function tsat

!-----------------------------------------------------------------------

recursive double precision function b23p(t)

  ! Returns the pressure on the boundary between regions 2 and 3,
  ! given a temperature t (deg C).

  implicit none
  double precision, intent(in):: t

  ! Local variable:
  double precision:: tk

  tk=t+tc_k
  b23p=1.d6*(nr23(1)+tk*(nr23(2)+tk*nr23(3)))

  return
end function b23p

!-----------------------------------------------------------------------

recursive double precision function b23t(p)

  ! Returns the temperature on the boundary between regions 2 and 3,
  ! given a pressure p (Pa).

  implicit none
  double precision, intent(in):: p

  b23t=nr23(4)+dsqrt((p/1.d6-nr23(5))/nr23(3))-tc_k

  return
end function b23t

!------------------------------------------------------------------------
end module IAPWS97
