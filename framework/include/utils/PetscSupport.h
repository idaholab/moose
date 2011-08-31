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

#ifndef PETSCSUPPORT_H
#define PETSCSUPPORT_H

#include "libmesh.h"

#ifdef LIBMESH_HAVE_PETSC

// libMesh
#include "petsc_nonlinear_solver.h"

class MProblem;

namespace Moose 
{
namespace PetscSupport
{
void petscSetOptions(const std::vector<std::string> & petsc_options,
                     const std::vector<std::string> & petsc_options_inames,
                     const std::vector<std::string> & petsc_options_values);

//    PetscErrorCode petscConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *dummy);
//    PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal pnorm,PetscReal fnorm,SNESConvergedReason *reason,void *dummy);
  
void petscSetDefaults(MProblem & problem);

void petscSetupDampers(NonlinearImplicitSystem& sys);

//    PetscErrorCode petscPhysicsBasedLineSearch(SNES snes,void *lsctx,Vec x,Vec f,Vec g,Vec y,Vec w, PetscReal fnorm,PetscReal *ynorm,PetscReal *gnorm,PetscTruth *flag);
//    PetscErrorCode dampedCheck(SNES snes, Vec x, Vec y, Vec w, void *lsctx, PetscTruth * changed_y, PetscTruth * changed_w);
}
}

#endif //LIBMESH_HAVE_PETSC

#endif //PETSCSUPPORT_H
