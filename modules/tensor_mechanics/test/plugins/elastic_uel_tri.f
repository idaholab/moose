! MIT License

! Copyright (c) 2020 irfancn

! Permission is hereby granted, free of charge, to any person obtaining a copy
! of this software and associated documentation files (the "Software"), to deal
! in the Software without restriction, including without limitation the rights
! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
! copies of the Software, and to permit persons to whom the Software is
! furnished to do so, subject to the following conditions:

! The above copyright notice and this permission notice shall be included in all
! copies or substantial portions of the Software.

! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
! SOFTWARE.

c ======================================================================
c User Subroutine UEL for linear elastic material
c All rights of reproduction or distribution in any form are reserved.
c By Irfan Habeeb CN (PhD, Technion - IIT)
c ======================================================================
      subroutine uel(rhs,amatrx,svars,energy,ndofel,nrhs,nsvars,
     1     props,nprops,coords,mcrd,nnode,u,du,v,a,jtype,time,dtime,
     2     kstep,kinc,jelem,params,ndload,jdltyp,adlmag,predef,
     3     npredf,lflags,mlvarx,ddlmag,mdload,pnewdt,jprops,njprop,
     4     period)

!   include 'aba_param.inc'

! ******************** state variables in the code *********************
! svars(1) = exx ( at the instant )
! svars(2) = eyy
! svars(3) = ezz (= 0)
! svars(4) = exy
! svars(5) = sxx
! svars(6) = syy
! svars(7) = szz (= 0)
! svars(8) = sxy
! **********************************************************************
      parameter(ngauss=1, nelem=185, nsdv=8)

      dimension rhs(mlvarx,*),amatrx(ndofel,ndofel),
     1     svars(nsvars),energy(8),props(*),coords(mcrd,nnode),
     2     u(ndofel),du(mlvarx,*),v(ndofel),a(ndofel),time(2),
     3     params(3),jdltyp(mdload,*),adlmag(mdload,*),
     4     ddlmag(mdload,*),predef(2,npredf,nnode),lflags(*),jprops(*)

      real*8 aintw(3),xii(2,3),xi(2),dNdxi(3,2),dxdxi(2,2),dxidx(2,2),
     *       dNdx(3,2),B(3,6),BT(6,3),eps(3),stn(4),stress(3),C(3,3),
     *       ddsdde(4,4),DB(3,6),sig2(3),sdv(nsvars), shape_f(3),str(3),
     *       dstran(4),B_phase(2,3),sig(4)
      real*8 E,nu,we,deter

      real*8 ceps(3),auxm3(3,3),B_B(3,3),aux(6),auxm2(6,3),stiff2(3,6),
     *       antn(3,3),F1(3,3),F2(3),F3(3),F4(3), dv(6), dT,
     *       epsceps,phase,aka,alc,gc,phase0,epsi,d_dot, lamda, mu
      integer I, J, K, k1, k2, k3

!   common/custom/uvars(nelem, 24, ngauss)
      ! DO K1=1, NPROPS
      !   WRITE(*,130) K1, PROPS(K1)
      ! END DO

      ! WRITE(*,131) MCRD
      ! WRITE(*,133) NNODE
      ! WRITE(*,142) nrhs

      ! DO K1=1, MCRD
      !   DO K2=1, NNODE
      !     WRITE(*,132) K1, K2, COORDS(K1, K2)
      !   END DO
      ! END DO

      ! WRITE(*,134) NDOFEL
      ! WRITE(*,135) MLVARX

      ! DO K1=1, NDOFEL
      !   WRITE(*,136) K1, U(K1)
      ! END DO

      ! DO K1=1, MLVARX
      !   WRITE(*,137) K1, 1, DU(K1,1)
      ! END DO

      ! DO K1=1, NDOFEL
      !   WRITE(*,138) K1, V(K1)
      ! END DO
      ! DO K1=1, NDOFEL
      !   WRITE(*,139) K1, A(K1)
      ! END DO

      ! WRITE(*,140) NSVARS
      ! DO K1=1, NSVARS
      !   WRITE(*,141) K1, SVARS(K1)
      ! END DO

      ! CALL FLUSH()

! 130   FORMAT ( 1X 'PROPS_', I2, :, 3X, 4F10.7 )
130   FORMAT ( 1X 'PROPS_', I2, :, 3X, F10.7 )
131   FORMAT ( 1X 'MCRD_', :,  2I2 )
132   FORMAT ( 1X 'COORDS_', I2, I2,:, 3X, 4F10.7 )
133   FORMAT ( 1X 'NNODE_', :,  2I2 )
134   FORMAT ( 1X 'NDOFEL_', :,  2I2 )
135   FORMAT ( 1X 'MLVARX_', :,  2I2 )
136   FORMAT ( 1X 'U_', I2, :, 3X, 4F10.7 )
137   FORMAT ( 1X 'DU_', I2, I2,:, 3X, 4F10.7 )
138   FORMAT ( 1X 'V_', I2, :, 3X, 4F10.7 )
139   FORMAT ( 1X 'A_', I2, :, 3X, 4F10.7 )
140   FORMAT ( 1X 'NSVARS_', :,  2I2 )
141   FORMAT ( 1X 'SVARS_', I2, :, 3X, 4F10.7 )
142   FORMAT ( 1X 'Nrhs_', :,  2I2 )

      E = props(1)
      nu = props(2)
      lamda = E*nu/((1.d0 + nu)*(1.d0 - 2.d0*nu))
      mu = E/(2.d0*(1.d0 + nu))

      do k1 = 1, ndofel
        do k2 = 1, nrhs
          rhs(k1, k2) = 0.d0
        end do
        do k2 = 1, ndofel
          amatrx(k2, k1) = 0.d0
        end do
      end do

      do k1 = 1, 3
        aintw(k1) = 0.5d0
        shape_f(k1) = 0.d0
      end do

      do k1 = 1, 2
        do k2 = 1, 3
          xii(k1, k2) = 1.d0/3.d0
        end do
      end do

      xi(1) = xii(1, 1)
      xi(2) = xii(2, 1)

      dNdxi = 0.d0
      dNdxi(1, 1) = 1.0
      dNdxi(2, 2) = 1.0
      dNdxi(3, 1) = -1.0
      dNdxi(3, 2) = -1.0

      do k1 = 1, 2
        do k2 = 1, 2
          dxdxi(k1, k2) = 0.d0
          do k3 = 1, nnode
            dxdxi(k1, k2) = dxdxi(k1, k2) + coords(k1,k3)*dNdxi(k3, k2)
          end do
        end do
      end do

      deter = 0.d0
      deter = dxdxi(1,1)*dxdxi(2,2) - dxdxi(1,2)*dxdxi(2,1)
      if (deter .lt. 0.d0) then
        print*, 'Negative Jacobian', deter
        call xit
      end if

      dxidx(1, 1) = dxdxi(2, 2)/deter
      dxidx(2, 2) = dxdxi(1, 1)/deter
      dxidx(1, 2) = -dxdxi(1, 2)/deter
      dxidx(2, 1) = -dxdxi(2, 1)/deter

      do k1 = 1, nnode
        do k2 = 1, 2
          dNdx(k1, k2) = 0.d0
          do k3 = 1, 2
            dNdx(k1, k2) = dNdx(k1, k2) + dNdxi(k1, k3)*dxidx(k3, k2)
          end do
        end do
      end do

      iy = 0
      do k1 = 1, nnode
        ix = iy + 1
        iy = ix + 1
        B(1, ix) = dNdx(k1, 1)
        B(1, iy) = 0.d0
        B(2, ix) = 0.d0
        B(2, iy) = dNdx(k1, 2)
        B(3, ix) = dNdx(k1, 2)
        B(3, iy) = dNdx(k1, 1)
      end do

      do k1 = 1, 3
        do k2 = 1, 6
          BT(k2, k1) = B(k1, k2)
        end do
      end do

      eps = 0.d0
      do k1 = 1, 3
        do k2 = 1, 6
          eps(k1) = eps(k1) + B(k1, k2) * DU(k2, 1)
        end do
      end do

      sdv = 0.d0
      do k1 = 1, nsdv
        sdv(k1) = svars(k1)
      end do

      C = 0.d0
      C(1, 1) = lamda + 2.d0*mu
      C(1, 2) = lamda
      C(2, 1) = C(1, 2)
      C(2, 2) = C(1, 1)
      C(3, 3) = mu

      stress = 0.d0
      do k1 = 1, 3
        do k2 = 1, 3
          stress(k2) = stress(k2) + C(k2, k1)*eps(k1)
        end do
      end do

      sig(1) = stress(1)
      sig(2) = stress(2)
      sig(3) = 0.d0
      sig(4) = stress(3)

      stn(1) = eps(1)
      stn(2) = eps(2)
      stn(3) = 0.d0
      stn(4) = eps(3)

      DB = 0.d0
      do k1 = 1, 3
        do k2 = 1, 6
          do k3 = 1, 3
            DB(k1, k2) = DB(k1, k2) + C(k1, k3)*B(k3, k2)
          end do
        end do
      end do

      we = 0.d0
      we = aintw(1) * dabs(deter)

      do k1 = 1, 6
        do k2 = 1, 3
          rhs(k1, 1) = rhs(k1, 1) - we*BT(k1, k2)*stress(k2)
        end do
      end do

      do k1 = 1, 6
        do k2 = 1, 6
          do k3 = 1, 3
            amatrx(k1, k2) = amatrx(k1, k2) + we*BT(k1, k3)*DB(k3, k2)
          end do
        end do
      end do

      sdv(1:4) = stn(1:4)             ! strain
      sdv(5:8) = sig(1:4)             ! stress
      svars(1:nsdv) = sdv(1:nsdv)
  !   uvars(jelem, 1:nsdv, 1) = sdv(1:nsdv)

      return
      end
