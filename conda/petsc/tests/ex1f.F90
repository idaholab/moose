!
!  Description: This example solves a nonlinear system on 1 processor with SNES.
!  We solve the  Bratu (SFI - solid fuel ignition) problem in a 2D rectangular
!  domain.  The command line options include:
!    -par <parameter>, where <parameter> indicates the nonlinearity of the problem
!       problem SFI:  <parameter> = Bratu parameter (0 <= par <= 6.81)
!    -mx <xg>, where <xg> = number of grid points in the x-direction
!    -my <yg>, where <yg> = number of grid points in the y-direction
!
!!/*T
!  Concepts: SNES^sequential Bratu example
!  Processors: 1
!T*/


!
!  --------------------------------------------------------------------------
!
!  Solid Fuel Ignition (SFI) problem.  This problem is modeled by
!  the partial differential equation
!
!          -Laplacian u - lambda*exp(u) = 0,  0 < x,y < 1,
!
!  with boundary conditions
!
!           u = 0  for  x = 0, x = 1, y = 0, y = 1.
!
!  A finite difference approximation with the usual 5-point stencil
!  is used to discretize the boundary value problem to obtain a nonlinear
!  system of equations.
!
!  The parallel version of this code is snes/examples/tutorials/ex5f.F
!
!  --------------------------------------------------------------------------

      program main
#include <petsc/finclude/petscdraw.h>
#include <petsc/finclude/petscsnes.h>
      use petscsnes
      implicit none
#if defined(PETSC_USING_F90) && !defined(PETSC_USE_FORTRANKIND)
      external SNESCOMPUTEJACOBIANDEFAULTCOLOR
#endif
!
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!                   Variable declarations
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!
!  Variables:
!     snes        - nonlinear solver
!     x, r        - solution, residual vectors
!     J           - Jacobian matrix
!     its         - iterations for convergence
!     matrix_free - flag - 1 indicates matrix-free version
!     lambda      - nonlinearity parameter
!     draw        - drawing context
!
      SNES               snes
      MatColoring        mc
      Vec                x,r
      PetscDraw               draw
      Mat                J
      PetscBool  matrix_free,flg,fd_coloring
      PetscErrorCode ierr
      PetscInt   its,N, mx,my,i5
      PetscMPIInt size,rank
      PetscReal   lambda_max,lambda_min,lambda
      MatFDColoring      fdcoloring
      ISColoring         iscoloring

      PetscScalar        lx_v(0:1)
      PetscOffset        lx_i

!  Store parameters in common block

      common /params/ lambda,mx,my,fd_coloring

!  Note: Any user-defined Fortran routines (such as FormJacobian)
!  MUST be declared as external.

      external FormFunction,FormInitialGuess,FormJacobian

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Initialize program
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      call PetscInitialize(PETSC_NULL_CHARACTER,ierr)
      if (ierr .ne. 0) then
        print*,'Unable to initialize PETSc'
        stop
      endif
      call MPI_Comm_size(PETSC_COMM_WORLD,size,ierr)
      call MPI_Comm_rank(PETSC_COMM_WORLD,rank,ierr)

      if (size .ne. 1) then
        SETERRA(PETSC_COMM_SELF,1,'This is a uniprocessor example only')
      endif

!  Initialize problem parameters
      i5 = 5
      lambda_max = 6.81
      lambda_min = 0.0
      lambda     = 6.0
      mx         = 4
      my         = 4
      call PetscOptionsGetInt(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,'-mx',mx,flg,ierr)
      call PetscOptionsGetInt(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,'-my',my,flg,ierr)
      call PetscOptionsGetReal(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,'-par',lambda,flg,ierr)
      if (lambda .ge. lambda_max .or. lambda .le. lambda_min) then
        SETERRA(PETSC_COMM_SELF,1,'Lambda out of range ');
      endif
      N       = mx*my

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Create nonlinear solver context
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      call SNESCreate(PETSC_COMM_WORLD,snes,ierr)

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Create vector data structures; set function evaluation routine
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      call VecCreate(PETSC_COMM_WORLD,x,ierr)
      call VecSetSizes(x,PETSC_DECIDE,N,ierr)
      call VecSetFromOptions(x,ierr)
      call VecDuplicate(x,r,ierr)

!  Set function evaluation routine and vector.  Whenever the nonlinear
!  solver needs to evaluate the nonlinear function, it will call this
!  routine.
!   - Note that the final routine argument is the user-defined
!     context that provides application-specific data for the
!     function evaluation routine.

      call SNESSetFunction(snes,r,FormFunction,fdcoloring,ierr)

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Create matrix data structure; set Jacobian evaluation routine
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

!  Create matrix. Here we only approximately preallocate storage space
!  for the Jacobian.  See the users manual for a discussion of better
!  techniques for preallocating matrix memory.

      call PetscOptionsHasName(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,'-snes_mf',matrix_free,ierr)
      if (.not. matrix_free) then
        call MatCreateSeqAIJ(PETSC_COMM_WORLD,N,N,i5,PETSC_NULL_INTEGER,J,ierr)
      endif

!
!     This option will cause the Jacobian to be computed via finite differences
!    efficiently using a coloring of the columns of the matrix.
!
      fd_coloring = .false.
      call PetscOptionsHasName(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,'-snes_fd_coloring',fd_coloring,ierr)
      if (fd_coloring) then

!
!      This initializes the nonzero structure of the Jacobian. This is artificial
!      because clearly if we had a routine to compute the Jacobian we won't need
!      to use finite differences.
!
        call FormJacobian(snes,x,J,J,0,ierr)
!
!       Color the matrix, i.e. determine groups of columns that share no common
!      rows. These columns in the Jacobian can all be computed simulataneously.
!
        call MatColoringCreate(J,mc,ierr)
        call MatColoringSetType(mc,MATCOLORINGNATURAL,ierr)
        call MatColoringSetFromOptions(mc,ierr)
        call MatColoringApply(mc,iscoloring,ierr)
        call MatColoringDestroy(mc,ierr)
!
!       Create the data structure that SNESComputeJacobianDefaultColor() uses
!       to compute the actual Jacobians via finite differences.
!
        call MatFDColoringCreate(J,iscoloring,fdcoloring,ierr)
        call MatFDColoringSetFunction(fdcoloring,FormFunction,fdcoloring,ierr)
        call MatFDColoringSetFromOptions(fdcoloring,ierr)
        call MatFDColoringSetUp(J,iscoloring,fdcoloring,ierr)
!
!        Tell SNES to use the routine SNESComputeJacobianDefaultColor()
!      to compute Jacobians.
!
        call SNESSetJacobian(snes,J,J,SNESComputeJacobianDefaultColor,fdcoloring,ierr)
        call ISColoringDestroy(iscoloring,ierr)

      else if (.not. matrix_free) then

!  Set Jacobian matrix data structure and default Jacobian evaluation
!  routine.  Whenever the nonlinear solver needs to compute the
!  Jacobian matrix, it will call this routine.
!   - Note that the final routine argument is the user-defined
!     context that provides application-specific data for the
!     Jacobian evaluation routine.
!   - The user can override with:
!      -snes_fd : default finite differencing approximation of Jacobian
!      -snes_mf : matrix-free Newton-Krylov method with no preconditioning
!                 (unless user explicitly sets preconditioner)
!      -snes_mf_operator : form preconditioning matrix as set by the user,
!                          but use matrix-free approx for Jacobian-vector
!                          products within Newton-Krylov method
!
        call SNESSetJacobian(snes,J,J,FormJacobian,0,ierr)
      endif

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Customize nonlinear solver; set runtime options
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

!  Set runtime options (e.g., -snes_monitor -snes_rtol <rtol> -ksp_type <type>)

      call SNESSetFromOptions(snes,ierr)

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Evaluate initial guess; then solve nonlinear system.
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

!  Note: The user should initialize the vector, x, with the initial guess
!  for the nonlinear solver prior to calling SNESSolve().  In particular,
!  to employ an initial guess of zero, the user should explicitly set
!  this vector to zero by calling VecSet().

      call FormInitialGuess(x,ierr)
      call SNESSolve(snes,PETSC_NULL_VEC,x,ierr)
      call SNESGetIterationNumber(snes,its,ierr);
      if (rank .eq. 0) then
         write(6,100) its
      endif
  100 format('Number of SNES iterations = ',i1)

!  PetscDraw contour plot of solution

      call PetscDrawCreate(PETSC_COMM_WORLD,PETSC_NULL_CHARACTER,'Solution',300,0,300,300,draw,ierr)
      call PetscDrawSetFromOptions(draw,ierr)

      call VecGetArrayRead(x,lx_v,lx_i,ierr)
      call PetscDrawTensorContour(draw,mx,my,PETSC_NULL_REAL,PETSC_NULL_REAL,lx_v(lx_i+1),ierr)
      call VecRestoreArrayRead(x,lx_v,lx_i,ierr)

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Free work space.  All PETSc objects should be destroyed when they
!  are no longer needed.
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      if (.not. matrix_free) call MatDestroy(J,ierr)
      if (fd_coloring) call MatFDColoringDestroy(fdcoloring,ierr)

      call VecDestroy(x,ierr)
      call VecDestroy(r,ierr)
      call SNESDestroy(snes,ierr)
      call PetscDrawDestroy(draw,ierr)
      call PetscFinalize(ierr)
      end

! ---------------------------------------------------------------------
!
!  FormInitialGuess - Forms initial approximation.
!
!  Input Parameter:
!  X - vector
!
!  Output Parameters:
!  X - vector
!  ierr - error code
!
!  Notes:
!  This routine serves as a wrapper for the lower-level routine
!  "ApplicationInitialGuess", where the actual computations are
!  done using the standard Fortran style of treating the local
!  vector data as a multidimensional array over the local mesh.
!  This routine merely accesses the local vector data via
!  VecGetArray() and VecRestoreArray().
!
      subroutine FormInitialGuess(X,ierr)
      use petscsnes
      implicit none

!  Input/output variables:
      Vec           X
      PetscErrorCode    ierr

!  Declarations for use with local arrays:
      PetscScalar   lx_v(0:1)
      PetscOffset   lx_i

      ierr   = 0

!  Get a pointer to vector data.
!    - For default PETSc vectors, VecGetArray() returns a pointer to
!      the data array.  Otherwise, the routine is implementation dependent.
!    - You MUST call VecRestoreArray() when you no longer need access to
!      the array.
!    - Note that the Fortran interface to VecGetArray() differs from the
!      C version.  See the users manual for details.

      call VecGetArray(X,lx_v,lx_i,ierr)

!  Compute initial guess

      call ApplicationInitialGuess(lx_v(lx_i),ierr)

!  Restore vector

      call VecRestoreArray(X,lx_v,lx_i,ierr)

      return
      end

! ---------------------------------------------------------------------
!
!  ApplicationInitialGuess - Computes initial approximation, called by
!  the higher level routine FormInitialGuess().
!
!  Input Parameter:
!  x - local vector data
!
!  Output Parameters:
!  f - local vector data, f(x)
!  ierr - error code
!
!  Notes:
!  This routine uses standard Fortran-style computations over a 2-dim array.
!
      subroutine ApplicationInitialGuess(x,ierr)
      use petscksp
      implicit none

!  Common blocks:
      PetscReal   lambda
      PetscInt     mx,my
      PetscBool         fd_coloring
      common      /params/ lambda,mx,my,fd_coloring

!  Input/output variables:
      PetscScalar x(mx,my)
      PetscErrorCode     ierr

!  Local variables:
      PetscInt     i,j
      PetscReal temp1,temp,hx,hy,one

!  Set parameters

      ierr   = 0
      one    = 1.0
      hx     = one/(mx-1)
      hy     = one/(my-1)
      temp1  = lambda/(lambda + one)

      do 20 j=1,my
         temp = min(j-1,my-j)*hy
         do 10 i=1,mx
            if (i .eq. 1 .or. j .eq. 1 .or. i .eq. mx .or. j .eq. my) then
              x(i,j) = 0.0
            else
              x(i,j) = temp1 * sqrt(min(min(i-1,mx-i)*hx,temp))
            endif
 10      continue
 20   continue

      return
      end

! ---------------------------------------------------------------------
!
!  FormFunction - Evaluates nonlinear function, F(x).
!
!  Input Parameters:
!  snes  - the SNES context
!  X     - input vector
!  dummy - optional user-defined context, as set by SNESSetFunction()
!          (not used here)
!
!  Output Parameter:
!  F     - vector with newly computed function
!
!  Notes:
!  This routine serves as a wrapper for the lower-level routine
!  "ApplicationFunction", where the actual computations are
!  done using the standard Fortran style of treating the local
!  vector data as a multidimensional array over the local mesh.
!  This routine merely accesses the local vector data via
!  VecGetArray() and VecRestoreArray().
!
      subroutine FormFunction(snes,X,F,fdcoloring,ierr)
      use petscsnes
      implicit none

!  Input/output variables:
      SNES              snes
      Vec               X,F
      PetscErrorCode          ierr
      MatFDColoring fdcoloring

!  Common blocks:
      PetscReal         lambda
      PetscInt          mx,my
      PetscBool         fd_coloring
      common            /params/ lambda,mx,my,fd_coloring

!  Declarations for use with local arrays:
      PetscScalar       lx_v(0:1),lf_v(0:1)
      PetscOffset       lx_i,lf_i

      PetscInt, pointer :: indices(:)

!  Get pointers to vector data.
!    - For default PETSc vectors, VecGetArray() returns a pointer to
!      the data array.  Otherwise, the routine is implementation dependent.
!    - You MUST call VecRestoreArray() when you no longer need access to
!      the array.
!    - Note that the Fortran interface to VecGetArray() differs from the
!      C version.  See the Fortran chapter of the users manual for details.

      call VecGetArrayRead(X,lx_v,lx_i,ierr)
      call VecGetArray(F,lf_v,lf_i,ierr)

!  Compute function

      call ApplicationFunction(lx_v(lx_i),lf_v(lf_i),ierr)

!  Restore vectors

      call VecRestoreArrayRead(X,lx_v,lx_i,ierr)
      call VecRestoreArray(F,lf_v,lf_i,ierr)

      call PetscLogFlops(11.0d0*mx*my,ierr)
!
!     fdcoloring is in the common block and used here ONLY to test the
!     calls to MatFDColoringGetPerturbedColumnsF90() and  MatFDColoringRestorePerturbedColumnsF90()
!
      if (fd_coloring) then
         call MatFDColoringGetPerturbedColumnsF90(fdcoloring,indices,ierr)
         print*,'Indices from GetPerturbedColumnsF90'
         print*,indices
         call MatFDColoringRestorePerturbedColumnsF90(fdcoloring,indices,ierr)
      endif
      return
      end

! ---------------------------------------------------------------------
!
!  ApplicationFunction - Computes nonlinear function, called by
!  the higher level routine FormFunction().
!
!  Input Parameter:
!  x    - local vector data
!
!  Output Parameters:
!  f    - local vector data, f(x)
!  ierr - error code
!
!  Notes:
!  This routine uses standard Fortran-style computations over a 2-dim array.
!
      subroutine ApplicationFunction(x,f,ierr)
      use petscsnes
      implicit none

!  Common blocks:
      PetscReal      lambda
      PetscInt        mx,my
      PetscBool         fd_coloring
      common         /params/ lambda,mx,my,fd_coloring

!  Input/output variables:
      PetscScalar    x(mx,my),f(mx,my)
      PetscErrorCode       ierr

!  Local variables:
      PetscScalar    two,one,hx,hy
      PetscScalar    hxdhy,hydhx,sc
      PetscScalar    u,uxx,uyy
      PetscInt        i,j

      ierr   = 0
      one    = 1.0
      two    = 2.0
      hx     = one/(mx-1)
      hy     = one/(my-1)
      sc     = hx*hy*lambda
      hxdhy  = hx/hy
      hydhx  = hy/hx

!  Compute function

      do 20 j=1,my
         do 10 i=1,mx
            if (i .eq. 1 .or. j .eq. 1 .or. i .eq. mx .or. j .eq. my) then
               f(i,j) = x(i,j)
            else
               u = x(i,j)
               uxx = hydhx * (two*u - x(i-1,j) - x(i+1,j))
               uyy = hxdhy * (two*u - x(i,j-1) - x(i,j+1))
               f(i,j) = uxx + uyy - sc*exp(u)
            endif
 10      continue
 20   continue

      return
      end

! ---------------------------------------------------------------------
!
!  FormJacobian - Evaluates Jacobian matrix.
!
!  Input Parameters:
!  snes    - the SNES context
!  x       - input vector
!  dummy   - optional user-defined context, as set by SNESSetJacobian()
!            (not used here)
!
!  Output Parameters:
!  jac      - Jacobian matrix
!  jac_prec - optionally different preconditioning matrix (not used here)
!  flag     - flag indicating matrix structure
!
!  Notes:
!  This routine serves as a wrapper for the lower-level routine
!  "ApplicationJacobian", where the actual computations are
!  done using the standard Fortran style of treating the local
!  vector data as a multidimensional array over the local mesh.
!  This routine merely accesses the local vector data via
!  VecGetArray() and VecRestoreArray().
!
      subroutine FormJacobian(snes,X,jac,jac_prec,dummy,ierr)
      use petscsnes
      implicit none

!  Input/output variables:
      SNES          snes
      Vec           X
      Mat           jac,jac_prec
      PetscErrorCode      ierr
      integer dummy

!  Common blocks:
      PetscReal     lambda
      PetscInt       mx,my
      PetscBool         fd_coloring
      common        /params/ lambda,mx,my,fd_coloring

!  Declarations for use with local array:
      PetscScalar   lx_v(0:1)
      PetscOffset   lx_i

!  Get a pointer to vector data

      call VecGetArrayRead(X,lx_v,lx_i,ierr)

!  Compute Jacobian entries

      call ApplicationJacobian(lx_v(lx_i),jac,jac_prec,ierr)

!  Restore vector

      call VecRestoreArrayRead(X,lx_v,lx_i,ierr)

!  Assemble matrix

      call MatAssemblyBegin(jac_prec,MAT_FINAL_ASSEMBLY,ierr)
      call MatAssemblyEnd(jac_prec,MAT_FINAL_ASSEMBLY,ierr)


      return
      end

! ---------------------------------------------------------------------
!
!  ApplicationJacobian - Computes Jacobian matrix, called by
!  the higher level routine FormJacobian().
!
!  Input Parameters:
!  x        - local vector data
!
!  Output Parameters:
!  jac      - Jacobian matrix
!  jac_prec - optionally different preconditioning matrix (not used here)
!  ierr     - error code
!
!  Notes:
!  This routine uses standard Fortran-style computations over a 2-dim array.
!
      subroutine ApplicationJacobian(x,jac,jac_prec,ierr)
      use petscsnes
      implicit none

!  Common blocks:
      PetscReal    lambda
      PetscInt      mx,my
      PetscBool         fd_coloring
      common       /params/ lambda,mx,my,fd_coloring

!  Input/output variables:
      PetscScalar  x(mx,my)
      Mat          jac,jac_prec
      PetscErrorCode      ierr

!  Local variables:
      PetscInt      i,j,row(1),col(5),i1,i5
      PetscScalar  two,one, hx,hy
      PetscScalar  hxdhy,hydhx,sc,v(5)

!  Set parameters

      i1     = 1
      i5     = 5
      one    = 1.0
      two    = 2.0
      hx     = one/(mx-1)
      hy     = one/(my-1)
      sc     = hx*hy
      hxdhy  = hx/hy
      hydhx  = hy/hx

!  Compute entries of the Jacobian matrix
!   - Here, we set all entries for a particular row at once.
!   - Note that MatSetValues() uses 0-based row and column numbers
!     in Fortran as well as in C.

      do 20 j=1,my
         row(1) = (j-1)*mx - 1
         do 10 i=1,mx
            row(1) = row(1) + 1
!           boundary points
            if (i .eq. 1 .or. j .eq. 1 .or. i .eq. mx .or. j .eq. my) then
               call MatSetValues(jac_prec,i1,row,i1,row,one,INSERT_VALUES,ierr)
!           interior grid points
            else
               v(1) = -hxdhy
               v(2) = -hydhx
               v(3) = two*(hydhx + hxdhy) - sc*lambda*exp(x(i,j))
               v(4) = -hydhx
               v(5) = -hxdhy
               col(1) = row(1) - mx
               col(2) = row(1) - 1
               col(3) = row(1)
               col(4) = row(1) + 1
               col(5) = row(1) + mx
               call MatSetValues(jac_prec,i1,row,i5,col,v,INSERT_VALUES,ierr)
            endif
 10      continue
 20   continue

      return
      end

!
!/*TEST
!
!   build:
!      requires: !single
!
!   test:
!      args: -snes_monitor_short -nox -snes_type newtontr -ksp_gmres_cgs_refinement_type refine_always
!
!   test:
!      suffix: 2
!      args: -snes_monitor_short -nox -snes_fd -ksp_gmres_cgs_refinement_type refine_always
!
!   test:
!      suffix: 3
!      args: -snes_monitor_short -nox -snes_fd_coloring -mat_coloring_type sl -ksp_gmres_cgs_refinement_type refine_always
!      filter: sort -b
!      filter_output: sort -b
!
!TEST*/
