
static char help[] = "Solves the nonlinear system, the Bratu (SFI - solid fuel ignition) problem in a 2D rectangular domain.\n\
This example also illustrates the use of matrix coloring.  Runtime options include:\n\
  -par <parameter>, where <parameter> indicates the problem's nonlinearity\n\
     problem SFI:  <parameter> = Bratu parameter (0 <= par <= 6.81)\n\
  -mx <xg>, where <xg> = number of grid points in the x-direction\n\
  -my <yg>, where <yg> = number of grid points in the y-direction\n\n";

/*T
   Concepts: SNES^sequential Bratu example
   Processors: 1
T*/



/* ------------------------------------------------------------------------

    Solid Fuel Ignition (SFI) problem.  This problem is modeled by
    the partial differential equation

            -Laplacian u - lambda*exp(u) = 0,  0 < x,y < 1,

    with boundary conditions

             u = 0  for  x = 0, x = 1, y = 0, y = 1.

    A finite difference approximation with the usual 5-point stencil
    is used to discretize the boundary value problem to obtain a nonlinear
    system of equations.

    The parallel version of this code is snes/examples/tutorials/ex5.c

  ------------------------------------------------------------------------- */

/*
   Include "petscsnes.h" so that we can use SNES solvers.  Note that
   this file automatically includes:
     petscsys.h       - base PETSc routines   petscvec.h - vectors
     petscmat.h - matrices
     petscis.h     - index sets            petscksp.h - Krylov subspace methods
     petscviewer.h - viewers               petscpc.h  - preconditioners
     petscksp.h   - linear solvers
*/

#include <petscsnes.h>

/*
   User-defined application context - contains data needed by the
   application-provided call-back routines, FormJacobian() and
   FormFunction().
*/
typedef struct {
  PetscReal param;              /* test problem parameter */
  PetscInt  mx;                 /* Discretization in x-direction */
  PetscInt  my;                 /* Discretization in y-direction */
} AppCtx;

/*
   User-defined routines
*/
extern PetscErrorCode FormJacobian(SNES,Vec,Mat,Mat,void*);
extern PetscErrorCode FormFunction(SNES,Vec,Vec,void*);
extern PetscErrorCode FormInitialGuess(AppCtx*,Vec);

int main(int argc,char **argv)
{
  SNES           snes;                 /* nonlinear solver context */
  Vec            x,r;                 /* solution, residual vectors */
  Mat            J;                    /* Jacobian matrix */
  AppCtx         user;                 /* user-defined application context */
  PetscErrorCode ierr;
  PetscInt       i,its,N,hist_its[50];
  PetscMPIInt    size;
  PetscReal      bratu_lambda_max = 6.81,bratu_lambda_min = 0.,history[50];
  MatFDColoring  fdcoloring;
  PetscBool      matrix_free = PETSC_FALSE,flg,fd_coloring = PETSC_FALSE;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help);if (ierr) return ierr;
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);
  if (size != 1) SETERRQ(PETSC_COMM_SELF,1,"This is a uniprocessor example only!");

  /*
     Initialize problem parameters
  */
  user.mx = 4; user.my = 4; user.param = 6.0;
  ierr    = PetscOptionsGetInt(NULL,NULL,"-mx",&user.mx,NULL);CHKERRQ(ierr);
  ierr    = PetscOptionsGetInt(NULL,NULL,"-my",&user.my,NULL);CHKERRQ(ierr);
  ierr    = PetscOptionsGetReal(NULL,NULL,"-par",&user.param,NULL);CHKERRQ(ierr);
  if (user.param >= bratu_lambda_max || user.param <= bratu_lambda_min) SETERRQ(PETSC_COMM_SELF,1,"Lambda is out of range");
  N = user.mx*user.my;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create nonlinear solver context
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ierr = SNESCreate(PETSC_COMM_WORLD,&snes);CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create vector data structures; set function evaluation routine
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ierr = VecCreate(PETSC_COMM_WORLD,&x);CHKERRQ(ierr);
  ierr = VecSetSizes(x,PETSC_DECIDE,N);CHKERRQ(ierr);
  ierr = VecSetFromOptions(x);CHKERRQ(ierr);
  ierr = VecDuplicate(x,&r);CHKERRQ(ierr);

  /*
     Set function evaluation routine and vector.  Whenever the nonlinear
     solver needs to evaluate the nonlinear function, it will call this
     routine.
      - Note that the final routine argument is the user-defined
        context that provides application-specific data for the
        function evaluation routine.
  */
  ierr = SNESSetFunction(snes,r,FormFunction,(void*)&user);CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create matrix data structure; set Jacobian evaluation routine
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*
     Create matrix. Here we only approximately preallocate storage space
     for the Jacobian.  See the users manual for a discussion of better
     techniques for preallocating matrix memory.
  */
  ierr = PetscOptionsGetBool(NULL,NULL,"-snes_mf",&matrix_free,NULL);CHKERRQ(ierr);
  if (!matrix_free) {
    PetscBool matrix_free_operator = PETSC_FALSE;
    ierr = PetscOptionsGetBool(NULL,NULL,"-snes_mf_operator",&matrix_free_operator,NULL);CHKERRQ(ierr);
    if (matrix_free_operator) matrix_free = PETSC_FALSE;
  }
  if (!matrix_free) {
    ierr = MatCreateSeqAIJ(PETSC_COMM_WORLD,N,N,5,NULL,&J);CHKERRQ(ierr);
  }

  /*
     This option will cause the Jacobian to be computed via finite differences
    efficiently using a coloring of the columns of the matrix.
  */
  ierr = PetscOptionsGetBool(NULL,NULL,"-snes_fd_coloring",&fd_coloring,NULL);CHKERRQ(ierr);
  if (matrix_free && fd_coloring) SETERRQ(PETSC_COMM_SELF,1,"Use only one of -snes_mf, -snes_fd_coloring options!\nYou can do -snes_mf_operator -snes_fd_coloring");

  if (fd_coloring) {
    ISColoring   iscoloring;
    MatColoring  mc;

    /*
      This initializes the nonzero structure of the Jacobian. This is artificial
      because clearly if we had a routine to compute the Jacobian we won't need
      to use finite differences.
    */
    ierr = FormJacobian(snes,x,J,J,&user);CHKERRQ(ierr);

    /*
       Color the matrix, i.e. determine groups of columns that share no common
      rows. These columns in the Jacobian can all be computed simulataneously.
    */
    ierr = MatColoringCreate(J,&mc);CHKERRQ(ierr);
    ierr = MatColoringSetType(mc,MATCOLORINGSL);CHKERRQ(ierr);
    ierr = MatColoringSetFromOptions(mc);CHKERRQ(ierr);
    ierr = MatColoringApply(mc,&iscoloring);CHKERRQ(ierr);
    ierr = MatColoringDestroy(&mc);CHKERRQ(ierr);
    /*
       Create the data structure that SNESComputeJacobianDefaultColor() uses
       to compute the actual Jacobians via finite differences.
    */
    ierr = MatFDColoringCreate(J,iscoloring,&fdcoloring);CHKERRQ(ierr);
    ierr = MatFDColoringSetFunction(fdcoloring,(PetscErrorCode (*)(void))FormFunction,&user);CHKERRQ(ierr);
    ierr = MatFDColoringSetFromOptions(fdcoloring);CHKERRQ(ierr);
    ierr = MatFDColoringSetUp(J,iscoloring,fdcoloring);CHKERRQ(ierr);
    /*
        Tell SNES to use the routine SNESComputeJacobianDefaultColor()
      to compute Jacobians.
    */
    ierr = SNESSetJacobian(snes,J,J,SNESComputeJacobianDefaultColor,fdcoloring);CHKERRQ(ierr);
    ierr = ISColoringDestroy(&iscoloring);CHKERRQ(ierr);
  }
  /*
     Set Jacobian matrix data structure and default Jacobian evaluation
     routine.  Whenever the nonlinear solver needs to compute the
     Jacobian matrix, it will call this routine.
      - Note that the final routine argument is the user-defined
        context that provides application-specific data for the
        Jacobian evaluation routine.
      - The user can override with:
         -snes_fd : default finite differencing approximation of Jacobian
         -snes_mf : matrix-free Newton-Krylov method with no preconditioning
                    (unless user explicitly sets preconditioner)
         -snes_mf_operator : form preconditioning matrix as set by the user,
                             but use matrix-free approx for Jacobian-vector
                             products within Newton-Krylov method
  */
  else if (!matrix_free) {
    ierr = SNESSetJacobian(snes,J,J,FormJacobian,(void*)&user);CHKERRQ(ierr);
  }

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Customize nonlinear solver; set runtime options
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*
     Set runtime options (e.g., -snes_monitor -snes_rtol <rtol> -ksp_type <type>)
  */
  ierr = SNESSetFromOptions(snes);CHKERRQ(ierr);

  /*
     Set array that saves the function norms.  This array is intended
     when the user wants to save the convergence history for later use
     rather than just to view the function norms via -snes_monitor.
  */
  ierr = SNESSetConvergenceHistory(snes,history,hist_its,50,PETSC_TRUE);CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Evaluate initial guess; then solve nonlinear system
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*
     Note: The user should initialize the vector, x, with the initial guess
     for the nonlinear solver prior to calling SNESSolve().  In particular,
     to employ an initial guess of zero, the user should explicitly set
     this vector to zero by calling VecSet().
  */
  ierr = FormInitialGuess(&user,x);CHKERRQ(ierr);
  ierr = SNESSolve(snes,NULL,x);CHKERRQ(ierr);
  ierr = SNESGetIterationNumber(snes,&its);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Number of SNES iterations = %D\n",its);CHKERRQ(ierr);


  /*
     Print the convergence history.  This is intended just to demonstrate
     use of the data attained via SNESSetConvergenceHistory().
  */
  ierr = PetscOptionsHasName(NULL,NULL,"-print_history",&flg);CHKERRQ(ierr);
  if (flg) {
    for (i=0; i<its+1; i++) {
      ierr = PetscPrintf(PETSC_COMM_WORLD,"iteration %D: Linear iterations %D Function norm = %g\n",i,hist_its[i],(double)history[i]);CHKERRQ(ierr);
    }
  }

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (!matrix_free) {
    ierr = MatDestroy(&J);CHKERRQ(ierr);
  }
  if (fd_coloring) {
    ierr = MatFDColoringDestroy(&fdcoloring);CHKERRQ(ierr);
  }
  ierr = VecDestroy(&x);CHKERRQ(ierr);
  ierr = VecDestroy(&r);CHKERRQ(ierr);
  ierr = SNESDestroy(&snes);CHKERRQ(ierr);
  ierr = PetscFinalize();
  return ierr;
}
/* ------------------------------------------------------------------- */
/*
   FormInitialGuess - Forms initial approximation.

   Input Parameters:
   user - user-defined application context
   X - vector

   Output Parameter:
   X - vector
 */
PetscErrorCode FormInitialGuess(AppCtx *user,Vec X)
{
  PetscInt       i,j,row,mx,my;
  PetscErrorCode ierr;
  PetscReal      lambda,temp1,temp,hx,hy;
  PetscScalar    *x;

  mx     = user->mx;
  my     = user->my;
  lambda = user->param;

  hx = 1.0 / (PetscReal)(mx-1);
  hy = 1.0 / (PetscReal)(my-1);

  /*
     Get a pointer to vector data.
       - For default PETSc vectors, VecGetArray() returns a pointer to
         the data array.  Otherwise, the routine is implementation dependent.
       - You MUST call VecRestoreArray() when you no longer need access to
         the array.
  */
  ierr  = VecGetArray(X,&x);CHKERRQ(ierr);
  temp1 = lambda/(lambda + 1.0);
  for (j=0; j<my; j++) {
    temp = (PetscReal)(PetscMin(j,my-j-1))*hy;
    for (i=0; i<mx; i++) {
      row = i + j*mx;
      if (i == 0 || j == 0 || i == mx-1 || j == my-1) {
        x[row] = 0.0;
        continue;
      }
      x[row] = temp1*PetscSqrtReal(PetscMin((PetscReal)(PetscMin(i,mx-i-1))*hx,temp));
    }
  }

  /*
     Restore vector
  */
  ierr = VecRestoreArray(X,&x);CHKERRQ(ierr);
  return 0;
}
/* ------------------------------------------------------------------- */
/*
   FormFunction - Evaluates nonlinear function, F(x).

   Input Parameters:
.  snes - the SNES context
.  X - input vector
.  ptr - optional user-defined context, as set by SNESSetFunction()

   Output Parameter:
.  F - function vector
 */
PetscErrorCode FormFunction(SNES snes,Vec X,Vec F,void *ptr)
{
  AppCtx            *user = (AppCtx*)ptr;
  PetscInt          i,j,row,mx,my;
  PetscErrorCode    ierr;
  PetscReal         two = 2.0,one = 1.0,lambda,hx,hy,hxdhy,hydhx;
  PetscScalar       ut,ub,ul,ur,u,uxx,uyy,sc,*f;
  const PetscScalar *x;

  mx     = user->mx;
  my     = user->my;
  lambda = user->param;
  hx     = one / (PetscReal)(mx-1);
  hy     = one / (PetscReal)(my-1);
  sc     = hx*hy;
  hxdhy  = hx/hy;
  hydhx  = hy/hx;

  /*
     Get pointers to vector data
  */
  ierr = VecGetArrayRead(X,&x);CHKERRQ(ierr);
  ierr = VecGetArray(F,&f);CHKERRQ(ierr);

  /*
     Compute function
  */
  for (j=0; j<my; j++) {
    for (i=0; i<mx; i++) {
      row = i + j*mx;
      if (i == 0 || j == 0 || i == mx-1 || j == my-1) {
        f[row] = x[row];
        continue;
      }
      u      = x[row];
      ub     = x[row - mx];
      ul     = x[row - 1];
      ut     = x[row + mx];
      ur     = x[row + 1];
      uxx    = (-ur + two*u - ul)*hydhx;
      uyy    = (-ut + two*u - ub)*hxdhy;
      f[row] = uxx + uyy - sc*lambda*PetscExpScalar(u);
    }
  }

  /*
     Restore vectors
  */
  ierr = VecRestoreArrayRead(X,&x);CHKERRQ(ierr);
  ierr = VecRestoreArray(F,&f);CHKERRQ(ierr);
  return 0;
}
/* ------------------------------------------------------------------- */
/*
   FormJacobian - Evaluates Jacobian matrix.

   Input Parameters:
.  snes - the SNES context
.  x - input vector
.  ptr - optional user-defined context, as set by SNESSetJacobian()

   Output Parameters:
.  A - Jacobian matrix
.  B - optionally different preconditioning matrix
.  flag - flag indicating matrix structure
*/
PetscErrorCode FormJacobian(SNES snes,Vec X,Mat J,Mat jac,void *ptr)
{
  AppCtx            *user = (AppCtx*)ptr;   /* user-defined applicatin context */
  PetscInt          i,j,row,mx,my,col[5];
  PetscErrorCode    ierr;
  PetscScalar       two = 2.0,one = 1.0,lambda,v[5],sc;
  const PetscScalar *x;
  PetscReal         hx,hy,hxdhy,hydhx;

  mx     = user->mx;
  my     = user->my;
  lambda = user->param;
  hx     = 1.0 / (PetscReal)(mx-1);
  hy     = 1.0 / (PetscReal)(my-1);
  sc     = hx*hy;
  hxdhy  = hx/hy;
  hydhx  = hy/hx;

  /*
     Get pointer to vector data
  */
  ierr = VecGetArrayRead(X,&x);CHKERRQ(ierr);

  /*
     Compute entries of the Jacobian
  */
  for (j=0; j<my; j++) {
    for (i=0; i<mx; i++) {
      row = i + j*mx;
      if (i == 0 || j == 0 || i == mx-1 || j == my-1) {
        ierr = MatSetValues(jac,1,&row,1,&row,&one,INSERT_VALUES);CHKERRQ(ierr);
        continue;
      }
      v[0] = -hxdhy; col[0] = row - mx;
      v[1] = -hydhx; col[1] = row - 1;
      v[2] = two*(hydhx + hxdhy) - sc*lambda*PetscExpScalar(x[row]); col[2] = row;
      v[3] = -hydhx; col[3] = row + 1;
      v[4] = -hxdhy; col[4] = row + mx;
      ierr = MatSetValues(jac,1,&row,5,col,v,INSERT_VALUES);CHKERRQ(ierr);
    }
  }

  /*
     Restore vector
  */
  ierr = VecRestoreArrayRead(X,&x);CHKERRQ(ierr);

  /*
     Assemble matrix
  */
  ierr = MatAssemblyBegin(jac,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(jac,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

  if (jac != J) {
    ierr = MatAssemblyBegin(J,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
    ierr = MatAssemblyEnd(J,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  }

  return 0;
}



/*TEST

   build:
      requires: !single

   test:
      args: -ksp_gmres_cgs_refinement_type refine_always

   test:
      suffix: 2
      args: -snes_monitor_short -snes_type newtontr -ksp_gmres_cgs_refinement_type refine_always

   test:
      suffix: 3
      args: -snes_monitor_short -mat_coloring_type sl -snes_fd_coloring -mx 8 -my 11 -ksp_gmres_cgs_refinement_type refine_always

TEST*/
