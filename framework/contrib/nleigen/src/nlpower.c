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
  Vec         y_tmp;
  Vec         res;
  SNES        snes;
  void *jacobianctxA;
  void *functionctxA;
  PetscErrorCode (*formFunctionA)(SNES, Vec, Vec, void *);
  PetscErrorCode (*formJacobianA)(SNES, Vec, Mat, Mat, void *);
  void *jacobianctxB;
  void *functionctxB;
  PetscErrorCode (*formFunctionB)(SNES, Vec, Vec, void *);
  PetscErrorCode (*formJacobianB)(SNES, Vec, Mat, Mat, void *);
  PetscBool initialized;
} EPS_NLPOWER;

#undef __FUNCT__
#define __FUNCT__ "EPSSetUp_NLPower"
PetscErrorCode EPSSetUp_NLPower(EPS eps)
{
  PetscErrorCode ierr;
  EPS_NLPOWER    *power = (EPS_NLPOWER *)eps->data;
  PetscBool      flg, istrivial;
  STMatMode      mode;

  PetscFunctionBegin;
  if (eps->ncv)
  {
    if (eps->ncv < eps->nev)
      SETERRQ(PetscObjectComm((PetscObject)eps), 1, "The value of ncv must be at least nev");
  } else eps->ncv = eps->nev;
  if (eps->mpd) {
    ierr = PetscInfo(eps, "Warning: parameter mpd ignored\n");CHKERRQ(ierr);
  }
  if (!eps->max_it) eps->max_it = PetscMax(2000, 100 * eps->n);

  /* The smallest eigenvalue is taken care by default */
  if (!eps->which)  eps->which = EPS_SMALLEST_MAGNITUDE;

  if (eps->which != EPS_SMALLEST_MAGNITUDE && eps->which != EPS_TARGET_MAGNITUDE)
    SETERRQ(PetscObjectComm((PetscObject)eps), 1, "Wrong value of eps->which");

  if (power->shift_type != EPS_POWER_SHIFT_CONSTANT) {
    ierr = PetscObjectTypeCompareAny((PetscObject)eps->st, &flg, STSINVERT, STCAYLEY, "");CHKERRQ(ierr);
    if (!flg) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "Variable shifts only allowed in shift-and-invert or Cayley ST");
    ierr = STGetMatMode(eps->st, &mode);CHKERRQ(ierr);
    if (mode == ST_MATMODE_INPLACE) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "ST matrix mode inplace does not work with variable shifts");
  }

  if (eps->extraction) {
    ierr = PetscInfo(eps, "Warning: extraction type ignored\n");CHKERRQ(ierr);
  }

  if (eps->balance != EPS_BALANCE_NONE) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "Balancing not supported in this solver");
  if (eps->arbitrary) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "Arbitrary selection of eigenpairs not supported in this solver");

  ierr = RGIsTrivial(eps->rg, &istrivial); CHKERRQ(ierr);
  if (!istrivial) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_SUP, "This solver does not support region filtering");

  ierr = EPSAllocateSolution(eps, 0);CHKERRQ(ierr);
  ierr = EPS_SetInnerProduct(eps);CHKERRQ(ierr);
  ierr = EPSSetWorkVecs(eps, 2);CHKERRQ(ierr);

  if (!power->snes) {
    ierr = SNESCreate(PetscObjectComm((PetscObject)eps), &power->snes);CHKERRQ(ierr);
  }
  /* to avoid slepc setup preconditioner too early */
  if (eps->st->P) eps->st->P = NULL;
  /* do not do any transform */
  eps->st->transform = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSNESInit_NLPower"

PetscErrorCode EPSSNESInit_NLPower(EPS eps, Vec x, Vec y) {
  PetscErrorCode ierr;
  Mat A, B;
  EPS_NLPOWER *nlpower = (EPS_NLPOWER *)eps->data;
  PetscContainer container;

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
      ierr = PetscObjectQueryFunction((PetscObject)B, "formFunctionB", &(nlpower->formFunctionB));CHKERRQ(ierr);
      ierr = PetscObjectQueryFunction((PetscObject)B, "formJacobianB", &(nlpower->formJacobianB));CHKERRQ(ierr);
      ierr = PetscObjectQuery((PetscObject)B, "AppCtx", (PetscObject *)&container);CHKERRQ(ierr);
      ierr = PetscContainerGetPointer(container, &nlpower->functionctxB);CHKERRQ(ierr);
      ierr = PetscContainerGetPointer(container, &nlpower->jacobianctxB);CHKERRQ(ierr);
    }
  } else {
    SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_NULL, "operators are NULL, please set operators properly \n");
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
  if (nlpower->formFunctionA && nlpower->formJacobianA) {
    ierr = SNESSetFunction(nlpower->snes, nlpower->res, nlpower->formFunctionA, nlpower->functionctxA);CHKERRQ(ierr);
    ierr = SNESSetJacobian(nlpower->snes, A, A, nlpower->formJacobianA, nlpower->jacobianctxA);CHKERRQ(ierr);
  } else SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_NULL, "did not set functions for evaluating Jacobian and residual \n");
  ierr = SNESSetFromOptions(nlpower->snes);CHKERRQ(ierr);
  ierr = SNESSetUp(nlpower->snes);CHKERRQ(ierr);
  nlpower->initialized = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSNESSolve_NLPower"

PetscErrorCode EPSSNESSolve_NLPower(EPS eps, Vec x, Vec y)
{
  PetscErrorCode ierr;
  EPS_NLPOWER *nlpower = (EPS_NLPOWER *)eps->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(eps, EPS_CLASSID, 1);
  PetscValidHeaderSpecific(x, VEC_CLASSID, 2);
  PetscValidHeaderSpecific(y, VEC_CLASSID, 3);
  if (x == y) SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_IDN, "x and y must be different vectors");
  if (!nlpower->initialized) {
    ierr = EPSSNESInit_NLPower(eps, x, y);CHKERRQ(ierr);
  }
  // Evaluate B*x
  ierr = nlpower->formFunctionB(nlpower->snes, x, nlpower->y_tmp, nlpower->functionctxB);CHKERRQ(ierr);
  ierr = VecCopy(x, y);CHKERRQ(ierr);
  ierr = SNESSolve(nlpower->snes, nlpower->y_tmp, y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSolve_NLPower"
PetscErrorCode EPSSolve_NLPower(EPS eps)
{
  PetscErrorCode ierr;
  EPS_NLPOWER *power = (EPS_NLPOWER *)eps->data;
  PetscInt k;
  Vec v, y, e;
  PetscReal relerr, norm;
  PetscScalar theta, rho, sigma;
  PetscBool breakdown;
  ST st;

  PetscFunctionBegin;
  y = eps->work[1];
  e = eps->work[0];

  ierr = EPSGetStartVector_Moose(eps, 0, NULL);CHKERRQ(ierr);
  ierr = STGetShift(eps->st, &sigma);CHKERRQ(ierr); /* original shift */
  rho = sigma;

  ierr = PetscPrintf(PETSC_COMM_WORLD, "EPSSolve_NLPower \n");CHKERRQ(ierr);

  while (eps->reason == EPS_CONVERGED_ITERATING) {
    eps->its++;
    k = eps->nconv;

    /* y = OP v */
    ierr = BVGetColumn(eps->V, k, &v);CHKERRQ(ierr);

    ierr = EPSSNESSolve_NLPower(eps, v, y);CHKERRQ(ierr);
    ierr = BVRestoreColumn(eps->V, k, &v);CHKERRQ(ierr);

    /* theta = (v,y)_B */
    ierr = BVSetActiveColumns(eps->V, k, k + 1);CHKERRQ(ierr);
    ierr = BVDotVec(eps->V, y, &theta);CHKERRQ(ierr);

    if (power->shift_type == EPS_POWER_SHIFT_CONSTANT) { /* direct & inverse iteration */

      /* approximate eigenvalue is the Rayleigh quotient */
      eps->eigr[eps->nconv] = theta;

      /* compute relative error as ||y-theta v||_2/|theta| */
      ierr = VecCopy(y, e);CHKERRQ(ierr);
      ierr = BVGetColumn(eps->V, k, &v);CHKERRQ(ierr);
      ierr = VecAXPY(e, -theta, v);CHKERRQ(ierr);
      ierr = BVRestoreColumn(eps->V, k, &v);CHKERRQ(ierr);
      ierr = VecNorm(e, NORM_2, &norm);CHKERRQ(ierr);
      relerr = norm / PetscAbsScalar(theta);

    } else {
      SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_NULL, "do not support shift type \n");
    }
    eps->errest[eps->nconv] = relerr;

    /* purge previously converged eigenvectors */
    ierr = BVInsertVec(eps->V, k, y);
    CHKERRQ(ierr);
    ierr = BVOrthogonalizeColumn(eps->V, k, NULL, &norm, NULL);
    CHKERRQ(ierr);
    ierr = BVScaleColumn(eps->V, k, 1.0 / norm);
    CHKERRQ(ierr);

    /* if relerr<tol, accept eigenpair */
    if (relerr < eps->tol) {
      eps->nconv = eps->nconv + 1;
      if (eps->nconv < eps->nev) {
        ierr = EPSGetStartVector_Moose(eps, eps->nconv, &breakdown);
        CHKERRQ(ierr);
        if (breakdown) {
          eps->reason = EPS_DIVERGED_BREAKDOWN;
          ierr = PetscInfo(eps, "Unable to generate more start vectors\n");
          CHKERRQ(ierr);
          break;
        }
      }
    }
    ierr = EPSMonitor(eps, eps->its, eps->nconv, eps->eigr, eps->eigi, eps->errest, eps->nconv + 1);CHKERRQ(ierr);
    ierr = (*eps->stopping)(eps, eps->its, eps->max_it, eps->nconv, eps->nev, &eps->reason, eps->stoppingctx);CHKERRQ(ierr);
  }
  ierr = EPSGetST(eps,&st);CHKERRQ(ierr);
  st->Astate[0] = ((PetscObject)st->A[0])->state;
  st->Astate[1] = ((PetscObject)st->A[1])->state;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSSetFromOptions_NLPower"
PetscErrorCode EPSSetFromOptions_NLPower(PetscOptionItems *PetscOptionsObject, EPS eps)
{
  PetscErrorCode ierr;
  EPS_NLPOWER   *power = (EPS_NLPOWER *)eps->data;
  PetscBool      flg;
  EPSPowerShiftType shift;

  PetscFunctionBegin;
  ierr = PetscOptionsHead(PetscOptionsObject, "EPS Power Options");CHKERRQ(ierr);
  ierr = PetscOptionsEnum("-eps_power_shift_type", "Shift type",
                       "EPSPowerSetShiftType", EPSPowerShiftTypes,
                       (PetscEnum)power->shift_type, (PetscEnum *)&shift, &flg);CHKERRQ(ierr);
  if (flg) {
    ierr = EPSPowerSetShiftType(eps, shift);CHKERRQ(ierr);
  }
  if (power->shift_type != EPS_POWER_SHIFT_CONSTANT) {
    ierr = STSetType(eps->st, STSINVERT);CHKERRQ(ierr);
  }
  ierr = PetscOptionsTail();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSPowerSetShiftType_NLPower"
static PetscErrorCode EPSPowerSetShiftType_NLPower(EPS eps, EPSPowerShiftType shift)
{
  EPS_NLPOWER *power = (EPS_NLPOWER *)eps->data;

  PetscFunctionBegin;
  switch (shift) {
  case EPS_POWER_SHIFT_CONSTANT:
  case EPS_POWER_SHIFT_RAYLEIGH:
  case EPS_POWER_SHIFT_WILKINSON:
    power->shift_type = shift;
    break;
  default:
    SETERRQ(PetscObjectComm((PetscObject)eps), PETSC_ERR_ARG_OUTOFRANGE,
            "Invalid shift type");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSPowerGetShiftType_NLPower"
static PetscErrorCode EPSPowerGetShiftType_NLPower(EPS eps,EPSPowerShiftType *shift)
{
  EPS_NLPOWER *power = (EPS_NLPOWER *)eps->data;

  PetscFunctionBegin;
  *shift = power->shift_type;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSDestroy_NLPower"
PetscErrorCode EPSDestroy_NLPower(EPS eps)
{
  EPS_NLPOWER *power = (EPS_NLPOWER *)eps->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (power->snes) {
    ierr = SNESDestroy(&power->snes);CHKERRQ(ierr);
  }
  if (power->res) {
    ierr = VecDestroy(&power->res);CHKERRQ(ierr);
  }
  if (power->y_tmp) {
    ierr = VecDestroy(&power->y_tmp);CHKERRQ(ierr);
  }
  ierr = PetscFree(eps->data);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerSetShiftType_C", NULL);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerGetShiftType_C", NULL);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSView_NLPower"
PetscErrorCode EPSView_NLPower(EPS eps, PetscViewer viewer)
{
  PetscErrorCode ierr;
  EPS_NLPOWER   *power = (EPS_NLPOWER *)eps->data;
  PetscBool      isascii;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)viewer, PETSCVIEWERASCII, &isascii);CHKERRQ(ierr);
  if (isascii) {
    ierr = PetscViewerASCIIPrintf(viewer, " Nonlinear Power: %s shifts\n", EPSPowerShiftTypes[power->shift_type]);CHKERRQ(ierr);
  }
  if (power->snes) {
    ierr = SNESView(power->snes, viewer);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSCreate_NLPower"
PETSC_EXTERN PetscErrorCode EPSCreate_NLPower(EPS eps)
{
  EPS_NLPOWER *ctx;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscNewLog(eps, &ctx);CHKERRQ(ierr);
  eps->data = (void *)ctx;

  eps->ops->setup = EPSSetUp_NLPower;
  eps->ops->solve = EPSSolve_NLPower;
  eps->ops->setfromoptions = EPSSetFromOptions_NLPower;
  eps->ops->destroy = EPSDestroy_NLPower;
  eps->ops->view = EPSView_NLPower;
  eps->ops->backtransform = NULL;
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerSetShiftType_C", EPSPowerSetShiftType_NLPower);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)eps, "EPSPowerGetShiftType_C", EPSPowerGetShiftType_NLPower);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#endif
