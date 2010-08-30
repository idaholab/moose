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
    
    PetscErrorCode petscPhysicsBasedLineSearch(SNES snes,void *lsctx,Vec x,Vec f,Vec g,Vec y,Vec w, PetscReal fnorm,PetscReal *ynorm,PetscReal *gnorm,PetscTruth *flag);
  }
}

namespace MoosePetscSupport = Moose::PetscSupport;

#endif //PETSCSUPPORT_H

#endif //LIBMESH_HAVE_PETSC
  
