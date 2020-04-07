//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <PETScDiffusionFDM.h>

/*
   u_t = uxx + uyy
   0 < x < 1, 0 < y < 1;
   At t=0: u(x,y) = exp(c*r*r*r), if r=PetscSqrtReal((x-.5)*(x-.5) + (y-.5)*(y-.5)) < .125
           u(x,y) = 0.0           if r >= .125


   Boundary conditions:
   Drichlet BC:
   At x=0, x=1, y=0, y=1: u = 0.0

   Neumann BC:
   At x=0, x=1: du(x,y,t)/dx = 0
   At y=0, y=1: du(x,y,t)/dy = 0
*/

PetscErrorCode
PETScExternalSolverCreate(MPI_Comm comm, TS * ts)
{
  DM da;

  PetscErrorCode ierr;

  PetscFunctionBeginUser;
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create distributed array (DMDA) to manage parallel grid and vectors
  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = DMDACreate2d(comm,
                      DM_BOUNDARY_NONE,
                      DM_BOUNDARY_NONE,
                      DMDA_STENCIL_STAR,
                      11,
                      11,
                      PETSC_DECIDE,
                      PETSC_DECIDE,
                      1,
                      1,
                      NULL,
                      NULL,
                      &da);
  CHKERRQ(ierr);
  ierr = DMSetFromOptions(da);
  CHKERRQ(ierr);
  ierr = DMSetUp(da);
  CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create timestepping solver context
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = TSCreate(comm, ts);
  CHKERRQ(ierr);
  ierr = TSSetProblemType(*ts, TS_NONLINEAR);
  CHKERRQ(ierr);
  ierr = TSSetType(*ts, TSBEULER);
  CHKERRQ(ierr);
  ierr = TSSetDM(*ts, da);
  CHKERRQ(ierr);
  ierr = DMDestroy(&da);
  CHKERRQ(ierr);
  ierr = TSSetIFunction(*ts, NULL, FormIFunction, nullptr);
  CHKERRQ(ierr);
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Set Jacobian evaluation routine
  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = TSSetIJacobian(*ts, NULL, NULL, FormIJacobian, NULL);
  CHKERRQ(ierr);
  /*
   * Make it consistent with the original solver
   * This can be changed during runtime via -ts_dt
   */
  ierr = TSSetTimeStep(*ts, 0.2);
  CHKERRQ(ierr);
  ierr = TSSetFromOptions(*ts);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode
PETScExternalSolverDestroy(TS ts)
{
  PetscErrorCode ierr;

  PetscFunctionBeginUser;
  ierr = TSDestroy(&ts);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*
 * This is a modified version of PETSc/src/ts/examples/tutorials/ex15.c
 * to demonstrate how MOOSE interact with an external solver package
 */
PetscErrorCode
externalPETScDiffusionFDMSolve(
    TS ts, Vec u0, Vec u, PetscReal dt, PetscReal time, PetscBool * converged)
{
  PetscErrorCode ierr;
  TSConvergedReason reason;
#if !PETSC_VERSION_LESS_THAN(3, 8, 0)
  PetscInt current_step;
#endif
  DM da;

  PetscFunctionBeginUser;
  PetscValidHeaderSpecific(ts, TS_CLASSID, 1);
  PetscValidType(ts, 1);
  PetscValidHeaderSpecific(u0, VEC_CLASSID, 2);
  PetscValidType(u0, 2);
  PetscValidHeaderSpecific(u, VEC_CLASSID, 3);
  PetscValidType(u, 3);
  PetscValidPointer(converged, 6);

  ierr = TSGetDM(ts, &da);
  CHKERRQ(ierr);

#if !PETSC_VERSION_LESS_THAN(3, 7, 0)
  PetscOptionsSetValue(NULL, "-ts_monitor", NULL);
  PetscOptionsSetValue(NULL, "-snes_monitor", NULL);
  PetscOptionsSetValue(NULL, "-ksp_monitor", NULL);
#else
  PetscOptionsSetValue("-ts_monitor", NULL);
  PetscOptionsSetValue("-snes_monitor", NULL);
  PetscOptionsSetValue("-ksp_monitor", NULL);
#endif

  /*ierr = TSSetMaxTime(ts,1.0);CHKERRQ(ierr);*/
  ierr = TSSetExactFinalTime(ts, TS_EXACTFINALTIME_STEPOVER);
  CHKERRQ(ierr);

  ierr = VecCopy(u0, u);
  CHKERRQ(ierr);

  ierr = TSSetSolution(ts, u);
  CHKERRQ(ierr);
  ierr = TSSetTimeStep(ts, dt);
  CHKERRQ(ierr);
  ierr = TSSetTime(ts, time - dt);
  CHKERRQ(ierr);
#if !PETSC_VERSION_LESS_THAN(3, 8, 0)
  ierr = TSGetStepNumber(ts, &current_step);
  CHKERRQ(ierr);
  ierr = TSSetMaxSteps(ts, current_step + 1);
  CHKERRQ(ierr);
#else
  SETERRQ(PetscObjectComm((PetscObject)ts), PETSC_ERR_SUP, "Require PETSc-3.8.x or higher ");
#endif
  /*  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Sets various TS parameters from user options
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = TSSetFromOptions(ts);
  CHKERRQ(ierr);
  /*
   * Let the passed-in dt take the priority
   */
  ierr = TSSetTimeStep(ts, dt);
  CHKERRQ(ierr);
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Solve nonlinear system
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = TSSolve(ts, u);
  CHKERRQ(ierr);

  ierr = TSGetConvergedReason(ts, &reason);
  CHKERRQ(ierr);
  *converged = reason > 0 ? PETSC_TRUE : PETSC_FALSE;

  PetscFunctionReturn(0);
}

/* --------------------------------------------------------------------- */
/*
  FormIFunction = Udot - RHSFunction
*/
PetscErrorCode
FormIFunction(TS ts, PetscReal /*t*/, Vec U, Vec Udot, Vec F, void * /*ctx*/)
{
  PetscErrorCode ierr;
  DM da;
  PetscInt i, j, Mx, My, xs, ys, xm, ym;
  PetscReal hx, hy, sx, sy;
  PetscScalar u, uxx, uyy, **uarray, **f, **udot;
  Vec localU;
  MPI_Comm comm;

  PetscFunctionBeginUser;
  ierr = PetscObjectGetComm((PetscObject)ts, &comm);
  ierr = TSGetDM(ts, &da);
  CHKERRQ(ierr);
  ierr = DMGetLocalVector(da, &localU);
  CHKERRQ(ierr);
  ierr = DMDAGetInfo(da,
                     PETSC_IGNORE,
                     &Mx,
                     &My,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE);
  CHKERRQ(ierr);

  hx = 1.0 / (PetscReal)(Mx - 1);
  sx = 1.0 / (hx * hx);
  hy = 1.0 / (PetscReal)(My - 1);
  sy = 1.0 / (hy * hy);

  /*
     Scatter ghost points to local vector,using the 2-step process
        DMGlobalToLocalBegin(),DMGlobalToLocalEnd().
     By placing code between these two statements, computations can be
     done while messages are in transition.
  */
  ierr = DMGlobalToLocalBegin(da, U, INSERT_VALUES, localU);
  CHKERRQ(ierr);
  ierr = DMGlobalToLocalEnd(da, U, INSERT_VALUES, localU);
  CHKERRQ(ierr);

  /* Get pointers to vector data */
  ierr = DMDAVecGetArrayRead(da, localU, &uarray);
  CHKERRQ(ierr);
  ierr = DMDAVecGetArray(da, F, &f);
  CHKERRQ(ierr);
  ierr = DMDAVecGetArray(da, Udot, &udot);
  CHKERRQ(ierr);

  /* Get local grid boundaries */
  ierr = DMDAGetCorners(da, &xs, &ys, NULL, &xm, &ym, NULL);
  CHKERRQ(ierr);

  /* Compute function over the locally owned part of the grid */
  for (j = ys; j < ys + ym; j++)
  {
    for (i = xs; i < xs + xm; i++)
    {
      /* Boundary conditions */
      if (i == 0 || j == 0 || i == Mx - 1 || j == My - 1)
      {
        if (PETSC_TRUE)
        {                         /* Drichlet BC */
          f[j][i] = uarray[j][i]; /* F = U */
        }
        else
        { /* Neumann BC */
          if (i == 0 && j == 0)
          { /* SW corner */
            f[j][i] = uarray[j][i] - uarray[j + 1][i + 1];
          }
          else if (i == Mx - 1 && j == 0)
          { /* SE corner */
            f[j][i] = uarray[j][i] - uarray[j + 1][i - 1];
          }
          else if (i == 0 && j == My - 1)
          { /* NW corner */
            f[j][i] = uarray[j][i] - uarray[j - 1][i + 1];
          }
          else if (i == Mx - 1 && j == My - 1)
          { /* NE corner */
            f[j][i] = uarray[j][i] - uarray[j - 1][i - 1];
          }
          else if (i == 0)
          { /* Left */
            f[j][i] = uarray[j][i] - uarray[j][i + 1];
          }
          else if (i == Mx - 1)
          { /* Right */
            f[j][i] = uarray[j][i] - uarray[j][i - 1];
          }
          else if (j == 0)
          { /* Bottom */
            f[j][i] = uarray[j][i] - uarray[j + 1][i];
          }
          else if (j == My - 1)
          { /* Top */
            f[j][i] = uarray[j][i] - uarray[j - 1][i];
          }
        }
      }
      else
      { /* Interior */
        u = uarray[j][i];
        /* 5-point stencil */
        uxx = (-2.0 * u + uarray[j][i - 1] + uarray[j][i + 1]);
        uyy = (-2.0 * u + uarray[j - 1][i] + uarray[j + 1][i]);
        if (PETSC_FALSE)
        {
          /* 9-point stencil: assume hx=hy */
          uxx = 2.0 * uxx / 3.0 + (0.5 * (uarray[j - 1][i - 1] + uarray[j - 1][i + 1] +
                                          uarray[j + 1][i - 1] + uarray[j + 1][i + 1]) -
                                   2.0 * u) /
                                      6.0;
          uyy = 2.0 * uyy / 3.0 + (0.5 * (uarray[j - 1][i - 1] + uarray[j - 1][i + 1] +
                                          uarray[j + 1][i - 1] + uarray[j + 1][i + 1]) -
                                   2.0 * u) /
                                      6.0;
        }
        f[j][i] = udot[j][i] - (uxx * sx + uyy * sy);
      }
    }
  }

  /* Restore vectors */
  ierr = DMDAVecRestoreArrayRead(da, localU, &uarray);
  CHKERRQ(ierr);
  ierr = DMDAVecRestoreArray(da, F, &f);
  CHKERRQ(ierr);
  ierr = DMDAVecRestoreArray(da, Udot, &udot);
  CHKERRQ(ierr);
  ierr = DMRestoreLocalVector(da, &localU);
  CHKERRQ(ierr);
  ierr = PetscLogFlops(11.0 * ym * xm);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/* --------------------------------------------------------------------- */
/*
  FormIJacobian() - Compute IJacobian = dF/dU + a dF/dUdot
  This routine is not used with option '-use_coloring'
*/
PetscErrorCode
FormIJacobian(
    TS ts, PetscReal /*t*/, Vec /*U*/, Vec /*Udot*/, PetscReal a, Mat J, Mat Jpre, void * /*ctx*/)
{
  PetscErrorCode ierr;
  PetscInt i, j, Mx, My, xs, ys, xm, ym, nc;
  DM da;
  MatStencil col[5], row;
  PetscScalar vals[5], hx, hy, sx, sy;

  PetscFunctionBeginUser;
  ierr = TSGetDM(ts, &da);
  CHKERRQ(ierr);
  ierr = DMDAGetInfo(da,
                     PETSC_IGNORE,
                     &Mx,
                     &My,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE);
  CHKERRQ(ierr);
  ierr = DMDAGetCorners(da, &xs, &ys, NULL, &xm, &ym, NULL);
  CHKERRQ(ierr);

  hx = 1.0 / (PetscReal)(Mx - 1);
  sx = 1.0 / (hx * hx);
  hy = 1.0 / (PetscReal)(My - 1);
  sy = 1.0 / (hy * hy);

  for (j = ys; j < ys + ym; j++)
  {
    for (i = xs; i < xs + xm; i++)
    {
      nc = 0;
      row.j = j;
      row.i = i;
      if (PETSC_TRUE && (i == 0 || i == Mx - 1 || j == 0 || j == My - 1))
      {
        col[nc].j = j;
        col[nc].i = i;
        vals[nc++] = 1.0;
      }
      else if (PETSC_FALSE && i == 0)
      { /* Left Neumann */
        col[nc].j = j;
        col[nc].i = i;
        vals[nc++] = 1.0;
        col[nc].j = j;
        col[nc].i = i + 1;
        vals[nc++] = -1.0;
      }
      else if (PETSC_FALSE && i == Mx - 1)
      { /* Right Neumann */
        col[nc].j = j;
        col[nc].i = i;
        vals[nc++] = 1.0;
        col[nc].j = j;
        col[nc].i = i - 1;
        vals[nc++] = -1.0;
      }
      else if (PETSC_FALSE && j == 0)
      { /* Bottom Neumann */
        col[nc].j = j;
        col[nc].i = i;
        vals[nc++] = 1.0;
        col[nc].j = j + 1;
        col[nc].i = i;
        vals[nc++] = -1.0;
      }
      else if (PETSC_FALSE && j == My - 1)
      { /* Top Neumann */
        col[nc].j = j;
        col[nc].i = i;
        vals[nc++] = 1.0;
        col[nc].j = j - 1;
        col[nc].i = i;
        vals[nc++] = -1.0;
      }
      else
      { /* Interior */
        col[nc].j = j - 1;
        col[nc].i = i;
        vals[nc++] = -sy;
        col[nc].j = j;
        col[nc].i = i - 1;
        vals[nc++] = -sx;
        col[nc].j = j;
        col[nc].i = i;
        vals[nc++] = 2.0 * (sx + sy) + a;
        col[nc].j = j;
        col[nc].i = i + 1;
        vals[nc++] = -sx;
        col[nc].j = j + 1;
        col[nc].i = i;
        vals[nc++] = -sy;
      }
      ierr = MatSetValuesStencil(Jpre, 1, &row, nc, col, vals, INSERT_VALUES);
      CHKERRQ(ierr);
    }
  }
  ierr = MatAssemblyBegin(Jpre, MAT_FINAL_ASSEMBLY);
  CHKERRQ(ierr);
  ierr = MatAssemblyEnd(Jpre, MAT_FINAL_ASSEMBLY);
  CHKERRQ(ierr);
  if (J != Jpre)
  {
    ierr = MatAssemblyBegin(J, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
    ierr = MatAssemblyEnd(J, MAT_FINAL_ASSEMBLY);
    CHKERRQ(ierr);
  }

  PetscFunctionReturn(0);
}

/* ------------------------------------------------------------------- */
PetscErrorCode
FormInitialSolution(TS ts, Vec U, void * /*ptr*/)
{
  DM da;
  PetscReal c = -30.0;
  PetscErrorCode ierr;
  PetscInt i, j, xs, ys, xm, ym, Mx, My;
  PetscScalar ** u;
  PetscReal hx, hy, x, y, r;

  PetscFunctionBeginUser;
  ierr = TSGetDM(ts, &da);
  CHKERRQ(ierr);
  ierr = DMDAGetInfo(da,
                     PETSC_IGNORE,
                     &Mx,
                     &My,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE,
                     PETSC_IGNORE);
  CHKERRQ(ierr);

  hx = 1.0 / (PetscReal)(Mx - 1);
  hy = 1.0 / (PetscReal)(My - 1);

  /* Get pointers to vector data */
  ierr = DMDAVecGetArray(da, U, &u);
  CHKERRQ(ierr);

  /* Get local grid boundaries */
  ierr = DMDAGetCorners(da, &xs, &ys, NULL, &xm, &ym, NULL);
  CHKERRQ(ierr);

  /* Compute function over the locally owned part of the grid */
  for (j = ys; j < ys + ym; j++)
  {
    y = j * hy;
    for (i = xs; i < xs + xm; i++)
    {
      x = i * hx;
      r = PetscSqrtReal((x - .5) * (x - .5) + (y - .5) * (y - .5));
      if (r < .125)
        u[j][i] = PetscExpReal(c * r * r * r);
      else
        u[j][i] = 0.0;
    }
  }

  /* Restore vectors */
  ierr = DMDAVecRestoreArray(da, U, &u);
  CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
