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

//libMesh includes
#include "petsc_vector.h"
#include "getpot.h"
#include "transient_system.h"
#include "petsc_nonlinear_solver.h"

class MooseSystem;
class Executioner;

namespace Moose 
{
  namespace PetscSupport
  {
    void petscParseOptions(GetPot & input_file);
  
    PetscErrorCode petscConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *dummy);
    PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal pnorm,PetscReal fnorm,SNESConvergedReason *reason,void *dummy);
    
    void petscSetDefaults(MooseSystem &moose_system, Executioner *executioner);
    
//    PetscErrorCode petscPhysicsBasedLineSearch(SNES snes,void *lsctx,Vec x,Vec f,Vec g,Vec y,Vec w, PetscReal fnorm,PetscReal *ynorm,PetscReal *gnorm,PetscTruth *flag);
    PetscErrorCode dampedCheck(SNES snes, Vec x, Vec y, Vec w, void *lsctx, PetscTruth * changed_y, PetscTruth * changed_w);

    PetscErrorCode petsc_snes_monitor(SNES snes, PetscInt its, PetscReal fnorm, void * dummy);
  }
}

namespace MoosePetscSupport = Moose::PetscSupport;

#endif //PETSCSUPPORT_H

#endif //LIBMESH_HAVE_PETSC
  
