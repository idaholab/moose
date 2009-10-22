#ifndef PETSCSUPPORT_H
#define PETSCSUPPORT_H

#include "libmesh.h"

#ifdef LIBMESH_HAVE_PETSC


//libMesh includes
#include "petsc_vector.h"
#include "getpot.h"
#include "transient_system.h"
#include "petsc_nonlinear_solver.h"

namespace Moose 
{
  namespace PetscSupport
  {
    extern PetscReal l_abs_step_tol;
    extern EquationSystems *_equation_system;
  
    void petscParseOptions(GetPot & input_file);
  
    PetscErrorCode petscConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *dummy);
    PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal pnorm,PetscReal fnorm,SNESConvergedReason *reason,void *dummy);
    void petscSetDefaults(EquationSystems * es, TransientNonlinearImplicitSystem & system, void (*compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar), void (*compute_residual) (const NumericVector<Number>& soln, NumericVector<Number>& residual));
    PetscErrorCode petscPhysicsBasedLineSearch(SNES snes,void *lsctx,Vec x,Vec f,Vec g,Vec y,Vec w, PetscReal fnorm,PetscReal *ynorm,PetscReal *gnorm,PetscTruth *flag);
  }
}

namespace MoosePetscSupport = Moose::PetscSupport;

#endif //PETSCSUPPORT_H

#endif //LIBMESH_HAVE_PETSC
  
