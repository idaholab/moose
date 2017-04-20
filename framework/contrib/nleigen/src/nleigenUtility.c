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
#include "nleigenUtility.h"

#undef __FUNCT__
#define __FUNCT__ "EPSGetOperators_Moose"
PetscErrorCode EPSGetOperators_Moose(EPS eps,Mat *A,Mat *B)
{
  PetscErrorCode ierr;
  ST             st;
  PetscInt       k;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(eps,EPS_CLASSID,1);
  ierr = EPSGetST(eps,&st);CHKERRQ(ierr);
  if (A) { *A = st->A[0]; st->Astate[0] = ((PetscObject)st->A[0])->state;}
  if (B) {
    ierr = STGetNumMatrices(st,&k);CHKERRQ(ierr);
    if (k==1) B = NULL;
    else {
      *B = st->A[1]; st->Astate[1] = ((PetscObject)st->A[1])->state;
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSGetStartVector_Moose"
/*
  This function is copied over from SLEPc because SLEPc claims this as an internal subroutine
*/
PetscErrorCode EPSGetStartVector_Moose(EPS eps,PetscInt i,PetscBool *breakdown)
{
  PetscErrorCode ierr;
  PetscReal      norm;
  PetscBool      lindep;
  Vec            w,z;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(eps,EPS_CLASSID,1);
  PetscValidLogicalCollectiveInt(eps,i,2);

  /* For the first step, use the first initial vector, otherwise a random one */
  if (i>0 || eps->nini==0) {
    ierr = BVSetRandomColumn(eps->V,i);CHKERRQ(ierr);
  }

  /* Force the vector to be in the range of OP for definite generalized problems */
  if (eps->ispositive || (eps->isgeneralized && eps->ishermitian)) {
    ierr = BVCreateVec(eps->V,&w);CHKERRQ(ierr);
    ierr = BVCopyVec(eps->V,i,w);CHKERRQ(ierr);
    ierr = BVGetColumn(eps->V,i,&z);CHKERRQ(ierr);
    ierr = STApply(eps->st,w,z);CHKERRQ(ierr);
    ierr = BVRestoreColumn(eps->V,i,&z);CHKERRQ(ierr);
    ierr = VecDestroy(&w);CHKERRQ(ierr);
  }

  /* Orthonormalize the vector with respect to previous vectors */
  ierr = BVOrthogonalizeColumn(eps->V,i,NULL,&norm,&lindep);CHKERRQ(ierr);
  if (breakdown) *breakdown = lindep;
  else if (lindep || norm == 0.0) {
    if (i==0) SETERRQ(PetscObjectComm((PetscObject)eps),1,"Initial vector is zero or belongs to the deflation space");
    else SETERRQ(PetscObjectComm((PetscObject)eps),1,"Unable to generate more start vectors");
  }
  ierr = BVScaleColumn(eps->V,i,1.0/norm);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "EPSCreateSubIS"

/*
   This is a toy function,

   TODO: a better  way to compute indices
*/
PetscErrorCode EPSCreateSubIS(Vec vec, IS *sub_is)
{
  PetscErrorCode ierr;
  PetscInt       localsize, low, high, i, is_local_size;
  PetscInt       *sub_indices;


  PetscFunctionBegin;

  ierr = VecGetLocalSize(vec, &localsize);CHKERRQ(ierr);
  ierr = VecGetOwnershipRange(vec, &low, &high);CHKERRQ(ierr);
  is_local_size = localsize/2;
  ierr = PetscCalloc1(is_local_size,&sub_indices);CHKERRQ(ierr);

  for (i=0; i<localsize; i+=2)
  {
    sub_indices[i/2] = low+i;
  }
  ierr = ISCreateGeneral(PetscObjectComm((PetscObject)vec),is_local_size,sub_indices,PETSC_OWN_POINTER,sub_is);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#endif
