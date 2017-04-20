/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "libmesh/libmesh_config.h"

#if LIBMESH_HAVE_SLEPC

#include <slepc/private/epsimpl.h> /*I "slepceps.h" I*/
#include <slepc/private/stimpl.h>  /*I "slepcst.h" I*/
#include <slepcblaslapack.h>
#include <petscsnes.h>
#include "nleigenUtility.h"

typedef struct {
  EPSPowerShiftType shift_type;
  Vec               y_tmp; /* temporary work vectors */
  Vec               res;
  SNES              snes;
  void *jacobianctxA;
  void *functionctxA;
  PetscErrorCode (*formFunctionA)(SNES, Vec, Vec, void *);
  PetscErrorCode (*formJacobianA)(SNES, Vec, Mat, Mat, void *);
  void *jacobianctxB;
  void *functionctxB;
  PetscErrorCode (*formFunctionB)(SNES, Vec, Vec, void *);
  PetscErrorCode (*formJacobianB)(SNES, Vec, Mat, Mat, void *);
  PetscBool      eps_composed; /* compose eps */
  PetscBool      initialed; /* initialize snes */
  PetscReal      dvalue;
  PetscInt       iterrefine;
  PetscInt       n_free_power;
  IS             eigen_sub_is;
  PetscBool      set_sub_eigen;
} EPS_MONOLITH;

#undef __FUNCT__
#define __FUNCT__ "EPSSetUp_Monolith"
PetscErrorCode EPSSetUp_Monolith(EPS eps)
{
  PetscErrorCode ierr;
  EPS_MONOLITH   *monolith = (EPS_MONOLITH *)eps->data;
  PetscBool      flg, istrivial;
  STMatMode      mode;

  PetscFunctionBegin;
  if (eps->ncv) {
    if (eps->ncv < eps->nev) SETERRQ(PetscObjectComm((PetscObject)eps), 1, "The value of ncv must be at least nev");
  } else eps->ncv = eps->nev;
  if (eps->mpd) {
    ierr = PetscInfo(eps, "Warning: parameter mpd ignored\n");CHKERRQ(ierr);
  }
  if (!eps->max_it) eps->max_it = PetscMax(2000, 100 * eps->n);

  /* The smallest eigenvalue is taken care by default */
  if (!eps->which)  eps->which = EPS_SMALLEST_MAGNITUDE;

  if (eps->which != EPS_SMALLEST_MAGNITUDE && eps->which != EPS_TARGET_MAGNITUDE)
    SETERRQ(PetscObjectComm((PetscObject)eps), 1, "Wrong value of eps->which");
  if (monolith->shift_type != EPS_POWER_SHIFT_CONSTANT) {
    ierr = PetscObjectTypeCompareAny((PetscObject)eps->st, &flg, STSINVERT, STCAYLEY, "");CHKERRQ(ierr);
    if (!flg) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "Variable shifts only allowed in shift-and-invert or Cayley ST");
    ierr = STGetMatMode(eps->st, &mode); CHKERRQ(ierr);
    if (mode == ST_MATMODE_INPLACE) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "ST matrix mode inplace does not work with variable shifts");
  }
  if (eps->extraction) {
    ierr = PetscInfo(eps, "Warning: extraction type ignored\n"); CHKERRQ(ierr);
  }
  if (eps->balance != EPS_BALANCE_NONE)
    SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "Balancing not supported in this solver");
  if (eps->arbitrary) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "Arbitrary selection of eigenpairs not supported in this solver");
  ierr = RGIsTrivial(eps->rg, &istrivial);CHKERRQ(ierr);
  if (!istrivial) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "This solver does not support region filtering");
  ierr = EPSAllocateSolution(eps, 0);CHKERRQ(ierr);
  ierr = EPS_SetInnerProduct(eps);CHKERRQ(ierr);
  ierr = EPSSetWorkVecs(eps, 2);CHKERRQ(ierr);
  if (!monolith->snes) {
    ierr = SNESCreate(PetscObjectComm((PetscObject)eps), &monolith->snes);CHKERRQ(ierr);
  }
  /* to avoid slepc setup preconditioner too early */
  if (eps->st->P) eps->st->P = NULL;
  eps->st->transform = PETSC_FALSE;
  monolith->dvalue = 0.0;
  ierr = PetscOptionsGetReal(PETSC_NULL,"","-adjutable_jacobian_coeffient",&monolith->dvalue,NULL);CHKERRQ(ierr);
  /* need to a refinement of the solver? */
  monolith->iterrefine = 1;
  ierr = PetscOptionsGetInt(PETSC_NULL,"","-monolith_newton_iterative_refine",&monolith->iterrefine,NULL);CHKERRQ(ierr);
  /* The number of free power iterations  */
  monolith->n_free_power = 5;
  ierr = PetscOptionsGetInt(PETSC_NULL,"","-monolith_n_free_power",&monolith->n_free_power,NULL);CHKERRQ(ierr);
  /* rank deficient system   */
  monolith->set_sub_eigen = PETSC_FALSE;
  ierr = PetscOptionsGetBool(PETSC_NULL,"","-monolith_set_sub_eigen",&monolith->set_sub_eigen,NULL);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSNESMonitor_Monolith"
/*
 A monitor to display eigenvalue
*/
PetscErrorCode EPSSNESMonitor_Monolith(SNES snes,PetscInt its,PetscReal fnorm,void *ctx)
{
  PetscErrorCode ierr;
  EPS             eps;

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject)snes, "eps", (PetscObject *)&eps);CHKERRQ(ierr);
  ierr = PetscPrintf(PetscObjectComm((PetscObject)snes),"%3D Eigenvalue[%D] %14.12e \n",its,eps->nconv,eps->eigr[eps->nconv]);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FormFunction_Monolith"

PetscErrorCode FormFunction_Monolith(SNES snes, Vec x, Vec y, void *ctx)
{
  PetscErrorCode ierr;
  EPS eps;
  EPS_MONOLITH *monolith = 0;
  PetscScalar lambda_right = 0;
  Vec sub_eigen_vec;

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject)snes, "eps", (PetscObject *)&eps);CHKERRQ(ierr);
  monolith = (EPS_MONOLITH *)eps->data;
  if (!monolith->y_tmp) {
    ierr = VecDuplicate(y, &monolith->y_tmp);CHKERRQ(ierr);
  }
  ierr = VecSet(monolith->y_tmp, 0.0);CHKERRQ(ierr);

  ierr = monolith->formFunctionB(monolith->snes, x, monolith->y_tmp, monolith->functionctxB);CHKERRQ(ierr);

  ierr = monolith->formFunctionA(monolith->snes, x, y, monolith->functionctxA);CHKERRQ(ierr);

  if (monolith->set_sub_eigen) {
    ierr = VecGetSubVector(monolith->y_tmp,  monolith->eigen_sub_is, &sub_eigen_vec);CHKERRQ(ierr);
    ierr = VecNorm(sub_eigen_vec, NORM_2, &lambda_right);CHKERRQ(ierr);
    eps->eigr[eps->nconv] = 1.0 / lambda_right;
    ierr = VecScale(sub_eigen_vec, eps->eigr[eps->nconv]);CHKERRQ(ierr);
    ierr = VecRestoreSubVector(monolith->y_tmp, monolith->eigen_sub_is, &sub_eigen_vec);CHKERRQ(ierr);
  } else {
    ierr = VecNorm(monolith->y_tmp, NORM_2, &lambda_right);CHKERRQ(ierr);
    eps->eigr[eps->nconv] = 1.0 / lambda_right;
    ierr = VecScale(monolith->y_tmp, eps->eigr[eps->nconv]);CHKERRQ(ierr);
  }

  ierr = VecAXPY(y, -1.0, monolith->y_tmp);CHKERRQ(ierr);

  ierr = VecNorm(y, NORM_2, &lambda_right);CHKERRQ(ierr);
  eps->errest[eps->nconv] = lambda_right;

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FormJacobian_Monolith"

PetscErrorCode FormJacobian_Monolith(SNES snes, Vec x, Mat J, Mat T, void *ctx)
{
  PetscErrorCode ierr;
  Mat A, B;
  EPS eps;
  EPS_MONOLITH *monolith = 0;

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject)snes, "eps", (PetscObject *)&eps);CHKERRQ(ierr);
  monolith = (EPS_MONOLITH *)eps->data;
  ierr = EPSGetOperators_Moose(eps, &A, &B);CHKERRQ(ierr);

  ierr = monolith->formJacobianA(monolith->snes, x, A, A, monolith->functionctxA);CHKERRQ(ierr);
  ierr = monolith->formJacobianB(monolith->snes, x, B, B, monolith->functionctxB);CHKERRQ(ierr);

  if (A != T) SETERRQ(PETSC_COMM_SELF, PETSC_ERR_ARG_INCOMP, "A != T \n");

  ierr = MatAXPY(T, -monolith->dvalue * eps->eigr[eps->nconv], B, SAME_NONZERO_PATTERN);CHKERRQ(ierr);

  ierr = MatAssemblyBegin(T, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(T, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  if (J != T) {
    ierr = MatAssemblyBegin(J, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
    ierr = MatAssemblyEnd(J, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSNESInit_Monolith"

PetscErrorCode EPSSNESInit_Monolith(EPS eps, Vec x, Vec y)
{
  PetscErrorCode ierr;
  Mat A, B;
  EPS_MONOLITH   *nlpower = (EPS_MONOLITH *)eps->data;
  SNESLineSearch linesearch;
  KSP            ksp;
  PetscContainer container;
  const char     *prefix;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(eps, EPS_CLASSID, 1);
  PetscValidHeaderSpecific(x, VEC_CLASSID, 2);
  PetscValidHeaderSpecific(y, VEC_CLASSID, 3);
  if (x == y) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_IDN, "x and y must be different vectors");
  ierr = EPSGetOperators_Moose(eps, &A, &B);CHKERRQ(ierr);
  if (A) {
    ierr = PetscObjectQueryFunction((PetscObject)A, "formFunctionA", &(nlpower->formFunctionA));CHKERRQ(ierr);
    ierr = PetscObjectQueryFunction((PetscObject)A, "formJacobianA", &(nlpower->formJacobianA));CHKERRQ(ierr);
    ierr = PetscObjectQuery((PetscObject)A, "AppCtx", (PetscObject *)&container);CHKERRQ(ierr);
    ierr = PetscContainerGetPointer(container, &nlpower->functionctxA);CHKERRQ(ierr);
    ierr = PetscContainerGetPointer(container, &nlpower->jacobianctxA);CHKERRQ(ierr);
    if (B) {
      ierr = PetscObjectQueryFunction((PetscObject)B, "formFunctionB",
                                      &(nlpower->formFunctionB));
      CHKERRQ(ierr);
      ierr = PetscObjectQueryFunction((PetscObject)B, "formJacobianB", &(nlpower->formJacobianB));CHKERRQ(ierr);
      ierr = PetscObjectQuery((PetscObject)B, "AppCtx", (PetscObject *)&container);CHKERRQ(ierr);
      ierr = PetscContainerGetPointer(container, &nlpower->functionctxB);CHKERRQ(ierr);
      ierr = PetscContainerGetPointer(container, &nlpower->jacobianctxB);CHKERRQ(ierr);
    }
  } else {
    SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_NULL, "operator is null, please set operators properly \n");
  }
  if (!nlpower->y_tmp) {
    ierr = VecDuplicate(y, &nlpower->y_tmp);CHKERRQ(ierr);
  }
  ierr = VecSet(nlpower->y_tmp, 0.0);CHKERRQ(ierr);
  if (!nlpower->res) {
    ierr = VecDuplicate(y, &nlpower->res);CHKERRQ(ierr);
  }
  ierr = VecSet(nlpower->res, 0.0);CHKERRQ(ierr);
  if (!nlpower->snes) {
    ierr = SNESCreate(PetscObjectComm((PetscObject)eps), &nlpower->snes);CHKERRQ(ierr);
  }
  if (!nlpower->eps_composed) {
    ierr = PetscObjectCompose((PetscObject)nlpower->snes, "eps", (PetscObject)eps);CHKERRQ(ierr);
    nlpower->eps_composed = PETSC_TRUE;
  }
  if (!nlpower->eigen_sub_is && nlpower->set_sub_eigen) {
    ierr = EPSCreateSubIS(nlpower->y_tmp,&nlpower->eigen_sub_is);CHKERRQ(ierr);
  }
  ierr = SNESSetFunction(nlpower->snes, nlpower->res, FormFunction_Monolith, nlpower->functionctxA);CHKERRQ(ierr);
  ierr = SNESSetJacobian(nlpower->snes, A, A, FormJacobian_Monolith,nlpower->jacobianctxA);CHKERRQ(ierr);
  ierr = SNESMonitorSet(nlpower->snes, EPSSNESMonitor_Monolith, NULL, NULL);CHKERRQ(ierr);
  ierr = SNESSetTolerances(nlpower->snes,1e-50,1e-6,1e-6,100,10000);CHKERRQ(ierr);
  ierr = SNESGetKSP(nlpower->snes,&ksp);CHKERRQ(ierr);
  ierr = KSPSetTolerances(ksp, 1e-4, 1e-50, 1e+2,1000);CHKERRQ(ierr);
  ierr = EPSGetOptionsPrefix(eps,&prefix);CHKERRQ(ierr);
  ierr = SNESSetOptionsPrefix(nlpower->snes, prefix);CHKERRQ(ierr);
  ierr = SNESGetLineSearch(nlpower->snes, &linesearch);CHKERRQ(ierr);
  ierr = SNESLineSearchSetType(linesearch, "basic");CHKERRQ(ierr);
  ierr = SNESSetFromOptions(nlpower->snes);CHKERRQ(ierr);
  ierr = SNESSetUp(nlpower->snes);CHKERRQ(ierr);

  nlpower->initialed = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSNESSolve_Monolith"

PetscErrorCode EPSSNESSolve_Monolith(EPS eps, Vec x, Vec y)
{
  PetscErrorCode ierr;
  Mat A, B;
  EPS_MONOLITH *nlpower = (EPS_MONOLITH *)eps->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(eps, EPS_CLASSID, 1);
  PetscValidHeaderSpecific(x, VEC_CLASSID, 2);
  PetscValidHeaderSpecific(y, VEC_CLASSID, 3);
  if (x == y) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_IDN, "x and y must be different vectors");
  ierr = EPSGetOperators_Moose(eps, &A, &B);CHKERRQ(ierr);
  if (!nlpower->initialed) {
    ierr = EPSSNESInit_Monolith(eps, x, y);CHKERRQ(ierr);
  }
  ierr = VecCopy(x, y);CHKERRQ(ierr);
  ierr = SNESSolve(nlpower->snes, NULL, y);CHKERRQ(ierr);
  /*ierr = nlpower->formJacobianA(nlpower->snes, y, A, A, nlpower->jacobianctxA);CHKERRQ(ierr);
  ierr = nlpower->formJacobianB(nlpower->snes, y, B, B, nlpower->jacobianctxB);CHKERRQ(ierr);*/
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSComputeInitialGuess_Monolith"
/*
 Use a nonlinear inverse power to compute an intial guess
*/
PetscErrorCode EPSComputeInitialGuess_Monolith(EPS eps)
{
  EPS            powereps;
  Mat            A,B;
  EPS_MONOLITH   *monolith = (EPS_MONOLITH *)eps->data;
  Vec            power_v, monolith_v;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = EPSCreate(PetscObjectComm((PetscObject)eps), &powereps);CHKERRQ(ierr);
  ierr = EPSGetOperators_Moose(eps, &A, &B);CHKERRQ(ierr);
  ierr = EPSSetType(powereps, "nlpower");CHKERRQ(ierr);
  ierr = EPSSetOperators(powereps, A, B);CHKERRQ(ierr);
  ierr = EPSSetTolerances(powereps, 1e-6, monolith->n_free_power);CHKERRQ(ierr);
  ierr = EPSSetOptionsPrefix(powereps, "initial_");CHKERRQ(ierr);
  ierr = EPSSetProblemType(powereps, EPS_GNHEP);CHKERRQ(ierr);
  ierr = EPSSetWhichEigenpairs(powereps, EPS_SMALLEST_MAGNITUDE);CHKERRQ(ierr);
  ierr = EPSSetFromOptions(powereps);CHKERRQ(ierr);
  ierr = EPSSolve(powereps);CHKERRQ(ierr);
  eps->eigr[0] = powereps->eigr[0];
  ierr = BVGetColumn(eps->V, 0, &monolith_v);CHKERRQ(ierr);
  ierr = BVGetColumn(powereps->V, 0, &power_v);CHKERRQ(ierr);
  ierr = VecCopy(power_v,monolith_v);CHKERRQ(ierr);
  ierr = VecNormalize(monolith_v, NULL);CHKERRQ(ierr);
  ierr = BVRestoreColumn(eps->V, 0, &monolith_v);CHKERRQ(ierr);
  ierr = BVRestoreColumn(powereps->V, 0, &power_v);CHKERRQ(ierr);
  ierr = EPSDestroy(&powereps);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "EPSSolve_Monolith"
PetscErrorCode EPSSolve_Monolith(EPS eps)
{
  PetscErrorCode ierr;
  PetscInt k;
  Vec v, y, e;
  PetscReal relerr, norm;
  PetscBool breakdown;
  EPS_MONOLITH *monolith = (EPS_MONOLITH *)eps->data;
  ST st;

  PetscFunctionBegin;
  y = eps->work[1];
  e = eps->work[0];

  ierr = EPSGetStartVector_Moose(eps, 0, NULL);CHKERRQ(ierr);
  /* compute an intial guess */
  ierr = EPSComputeInitialGuess_Monolith(eps);CHKERRQ(ierr);

  while (eps->reason == EPS_CONVERGED_ITERATING || eps->its < monolith->iterrefine) {
    eps->its++;
    k = eps->nconv;

    /* y = OP v */
    ierr = BVGetColumn(eps->V, k, &v);CHKERRQ(ierr);
    ierr = EPSSNESSolve_Monolith(eps, v, y);CHKERRQ(ierr);
    ierr = BVRestoreColumn(eps->V, k, &v);CHKERRQ(ierr);

    ierr = BVSetActiveColumns(eps->V, k, k + 1);CHKERRQ(ierr);

    relerr = eps->errest[eps->nconv];

    /* purge previously converged eigenvectors */
    ierr = BVInsertVec(eps->V, k, y);CHKERRQ(ierr);
    /* if relerr<tol, accept eigenpair */
    if (relerr < eps->tol) {
      eps->nconv = eps->nconv + 1;
      if (eps->nconv < eps->nev) {
        ierr = EPSGetStartVector_Moose(eps, eps->nconv, &breakdown);CHKERRQ(ierr);
        if (breakdown) {
          eps->reason = EPS_DIVERGED_BREAKDOWN;
          ierr = PetscInfo(eps, "Unable to generate more start vectors\n");CHKERRQ(ierr);
          break;
        }
      }
    }
    ierr = EPSMonitor(eps, eps->its, eps->nconv, eps->eigr, eps->eigi,
                      eps->errest, eps->nconv + 1);CHKERRQ(ierr);
    ierr = (*eps->stopping)(eps, eps->its, eps->max_it, eps->nconv, eps->nev,
                            &eps->reason, eps->stoppingctx);CHKERRQ(ierr);
  }
  if (monolith->eps_composed) {
    ierr = PetscObjectCompose((PetscObject)(monolith->snes), "eps", NULL);CHKERRQ(ierr);
    monolith->eps_composed = PETSC_FALSE;
  }
  ierr = EPSGetST(eps,&st);CHKERRQ(ierr);
  st->Astate[0] = ((PetscObject)st->A[0])->state;
  st->Astate[1] = ((PetscObject)st->A[1])->state;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSetFromOptions_Monolith"
PetscErrorCode EPSSetFromOptions_Monolith(PetscOptionItems *PetscOptionsObject, EPS eps)
{
  PetscErrorCode ierr;
  EPS_MONOLITH *monolith = (EPS_MONOLITH *)eps->data;
  PetscBool flg;
  EPSPowerShiftType shift;

  PetscFunctionBegin;
  ierr = PetscOptionsHead(PetscOptionsObject, "EPS Power Options");CHKERRQ(ierr);
  ierr = PetscOptionsEnum("-eps_power_shift_type", "Shift type",
                          "EPSPowerSetShiftType", EPSPowerShiftTypes,
                          (PetscEnum)monolith->shift_type, (PetscEnum *)&shift,
                          &flg);CHKERRQ(ierr);
  if (flg) {
    ierr = EPSPowerSetShiftType(eps, shift);CHKERRQ(ierr);
  }
  if (monolith->shift_type != EPS_POWER_SHIFT_CONSTANT) {
    ierr = STSetType(eps->st, STSINVERT);CHKERRQ(ierr);
  }
  ierr = PetscOptionsTail();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSPowerSetShiftType_Monolith"
static PetscErrorCode EPSPowerSetShiftType_Monolith(EPS eps, EPSPowerShiftType shift)
{
  EPS_MONOLITH *monolith = (EPS_MONOLITH *)eps->data;

  PetscFunctionBegin;
  switch (shift) {
  case EPS_POWER_SHIFT_CONSTANT:
  case EPS_POWER_SHIFT_RAYLEIGH:
  case EPS_POWER_SHIFT_WILKINSON:
    monolith->shift_type = shift;
    break;
  default:
    SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_OUTOFRANGE,"Invalid shift type");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSPowerGetShiftType_Monolith"
static PetscErrorCode EPSPowerGetShiftType_Monolith(EPS eps, EPSPowerShiftType *shift)
{
  EPS_MONOLITH *monolith = (EPS_MONOLITH *)eps->data;

  PetscFunctionBegin;
  *shift = monolith->shift_type;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSDestroy_Monolith"
PetscErrorCode EPSDestroy_Monolith(EPS eps)
{
  EPS_MONOLITH *monolith = (EPS_MONOLITH *)eps->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (monolith->snes) {
    ierr = SNESDestroy(&monolith->snes);CHKERRQ(ierr);
  }
  if (monolith->res) {
    ierr = VecDestroy(&monolith->res);CHKERRQ(ierr);
  }
  if (monolith->y_tmp) {
    ierr = VecDestroy(&monolith->y_tmp);CHKERRQ(ierr);
  }
  if (monolith->eigen_sub_is) {
    ierr = ISDestroy(&monolith->eigen_sub_is);CHKERRQ(ierr);
  }
  ierr = PetscFree(eps->data);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerSetShiftType_C", NULL);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerGetShiftType_C", NULL);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSView_Monolith"
PetscErrorCode EPSView_Monolith(EPS eps, PetscViewer viewer)
{
  PetscErrorCode ierr;
  EPS_MONOLITH *monolith = (EPS_MONOLITH *)eps->data;
  PetscBool isascii;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)viewer, PETSCVIEWERASCII, &isascii);CHKERRQ(ierr);
  if (isascii) {
    ierr = PetscViewerASCIIPrintf(viewer, "  Monolith newton: %s shifts\n",
                                  EPSPowerShiftTypes[monolith->shift_type]);CHKERRQ(ierr);
  }
  if (monolith->snes) {
    ierr = SNESView(monolith->snes, viewer);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSCreate_Monolith"
PETSC_EXTERN PetscErrorCode EPSCreate_Monolith(EPS eps)
{
  EPS_MONOLITH *ctx;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscNewLog(eps, &ctx);CHKERRQ(ierr);
  eps->data = (void *)ctx;

  eps->ops->setup = EPSSetUp_Monolith;
  eps->ops->solve = EPSSolve_Monolith;
  eps->ops->setfromoptions = EPSSetFromOptions_Monolith;
  eps->ops->destroy = EPSDestroy_Monolith;
  eps->ops->view = EPSView_Monolith;
  eps->ops->backtransform = NULL;
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerSetShiftType_C", EPSPowerSetShiftType_Monolith);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerGetShiftType_C", EPSPowerGetShiftType_Monolith);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#endif
