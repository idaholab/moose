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

#include "PetscSupport.h"

#ifdef LIBMESH_HAVE_PETSC

#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
#include "DisplacedProblem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "MooseTypes.h"

//libMesh Includes
#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_linear_solver.h"
#include "libmesh/petsc_preconditioner.h"
#include "libmesh/getpot.h"

//PETSc includes
#include <petsc.h>
#include <petscsnes.h>
#include <petscksp.h>

#if PETSC_VERSION_LESS_THAN(3,3,0)
// PETSc 3.2.x and lower
#include <private/kspimpl.h>
#include <private/snesimpl.h>
#else
// PETSc 3.3.0+
#include <petsc-private/kspimpl.h>
#include <petsc-private/snesimpl.h>
#include <petscdm.h>
#endif


namespace Moose
{
namespace PetscSupport
{


void petscSetOptions(Problem & problem)
{
  const std::vector<MooseEnum>   & petsc_options = problem.parameters().get<std::vector<MooseEnum> >("petsc_options");
  const std::vector<std::string> & petsc_options_inames = problem.parameters().get<std::vector<std::string> >("petsc_inames");
  const std::vector<std::string> & petsc_options_values = problem.parameters().get<std::vector<std::string> >("petsc_values");

  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("Petsc names and options are not the same length");

  PetscOptionsClear();

  { // Get any options specified on the command-line
    int argc;
    char ** args;

    PetscGetArgs(&argc, &args);
    PetscOptionsInsert(&argc, &args, NULL);
  }

  // Add any options specified in the input file
  for (unsigned int i=0; i<petsc_options.size(); ++i)
    PetscOptionsSetValue(std::string(petsc_options[i]).c_str(), PETSC_NULL);

  for (unsigned int i=0; i<petsc_options_inames.size(); ++i)
    PetscOptionsSetValue(petsc_options_inames[i].c_str(), petsc_options_values[i].c_str());
}

PetscErrorCode  petscConverged(KSP ksp,PetscInt n,PetscReal rnorm,KSPConvergedReason *reason,void *dummy)
{
  FEProblem & problem = *static_cast<FEProblem *>(dummy);
  NonlinearSystem & system = problem.getNonlinearSystem();

  *reason = KSP_CONVERGED_ITERATING;

  //If it's the beginning of a new set of iterations, reset last_rnorm
  if (!n)
    system._last_rnorm = 1e99;

  PetscReal norm_diff = std::fabs(rnorm - system._last_rnorm);

  if(norm_diff < system._l_abs_step_tol)
  {
    *reason = KSP_CONVERGED_RTOL;
    return(0);
  }

  system._last_rnorm = rnorm;

  // From here, we want the default behavior of the KSPDefaultConverged
  // test, but we don't want PETSc to die in that function with a
  // CHKERRQ call... therefore we Push/Pop a different error handler
  // and then call KSPDefaultConverged().  Finally, if we hit the
  // max iteration count, we want to set KSP_CONVERGED_ITS.
  PetscPushErrorHandler(PetscReturnErrorHandler,/* void* ctx= */PETSC_NULL);

  // As of PETSc 3.0.0, you must call KSPDefaultConverged with a
  // non-NULL context pointer which must be created with
  // KSPDefaultConvergedCreate(), and destroyed with
  // KSPDefaultConvergedDestroy().
  /*PetscErrorCode ierr = */
  KSPDefaultConverged(ksp, n, rnorm, reason, dummy);

  // Pop the Error handler we pushed on the stack to go back
  // to default PETSc error handling behavior.
  PetscPopErrorHandler();

  // If we hit max its then we consider that converged
  if (n >= ksp->max_it) *reason = KSP_CONVERGED_ITS;

  if(*reason == KSP_CONVERGED_ITS || *reason == KSP_CONVERGED_RTOL)
    system._current_l_its.push_back(n);

  return 0;
}

PetscErrorCode petscNonlinearConverged(SNES snes,PetscInt it,PetscReal xnorm,PetscReal snorm,PetscReal fnorm,SNESConvergedReason *reason,void * dummy)
{

  // xnorm: norm of current iterate
  // pnorm (snorm): norm of
  // fnorm: norm of function
  FEProblem & problem = *static_cast<FEProblem *>(dummy);
  NonlinearSystem & system = problem.getNonlinearSystem();

#if PETSC_VERSION_LESS_THAN(3,3,0)
  PetscInt stol = snes->xtol;
#else
  PetscInt stol = snes->stol;
#endif
  std::string msg;

  const Real ref_resid = system._initial_residual;
  const Real div_threshold = system._initial_residual*(1.0/snes->rtol);

  MooseNonlinearConvergenceReason moose_reason = problem.checkNonlinearConvergence(msg,
                                                                                   it,
                                                                                   xnorm,
                                                                                   snorm,
                                                                                   fnorm,
                                                                                   snes->ttol,
                                                                                   snes->rtol,
                                                                                   stol,
                                                                                   snes->abstol,
                                                                                   snes->nfuncs,
                                                                                   snes->max_funcs,
                                                                                   ref_resid,
                                                                                   div_threshold);

  if (msg.length() > 0)
    PetscInfo(snes,msg.c_str());

  switch (moose_reason)
  {
    case MOOSE_ITERATING:
      *reason = SNES_CONVERGED_ITERATING;
      break;

    case MOOSE_CONVERGED_FNORM_ABS:
      *reason = SNES_CONVERGED_FNORM_ABS;
      break;

    case MOOSE_CONVERGED_FNORM_RELATIVE:
      *reason = SNES_CONVERGED_FNORM_RELATIVE;
      break;

    case MOOSE_CONVERGED_SNORM_RELATIVE:
#if PETSC_VERSION_LESS_THAN(3,3,0)
      *reason = SNES_CONVERGED_PNORM_RELATIVE;
#else
      *reason = SNES_CONVERGED_SNORM_RELATIVE;
#endif
      break;

    case MOOSE_DIVERGED_FUNCTION_COUNT:
      *reason = SNES_DIVERGED_FUNCTION_COUNT;
      break;

    case MOOSE_DIVERGED_FNORM_NAN:
      *reason = SNES_DIVERGED_FNORM_NAN;
      break;

    case MOOSE_DIVERGED_LINE_SEARCH:
#if PETSC_VERSION_LESS_THAN(3,2,0)
      *reason = SNES_DIVERGED_LS_FAILURE;
#else
      *reason = SNES_DIVERGED_LINE_SEARCH;
#endif
      break;
  }
  return (0);
}

#if PETSC_VERSION_LESS_THAN(3,3,0)
// PETSc 3.2.x-
PetscErrorCode dampedCheck(SNES /*snes*/, Vec x, Vec y, Vec w, void *lsctx, PetscBool * /*changed_y*/, PetscBool * changed_w)
#else
// PETSc 3.3.0+
  PetscErrorCode dampedCheck(SNESLineSearch /* linesearch */, Vec x, Vec y, Vec w, PetscBool * /*changed_y*/, PetscBool * changed_w, void *lsctx)
#endif
{
  // From SNESLineSearchSetPostCheck docs:
  // +  x - old solution vector
  // .  y - search direction vector
  // .  w - new solution vector  w = x-y
  // .  changed_y - indicates that the line search changed y
  // .  changed_w - indicates that the line search changed w

  int ierr = 0;
  Real damping = 1.0;

  FEProblem & problem = *static_cast<FEProblem *>(lsctx);

  TransientNonlinearImplicitSystem & system = problem.getNonlinearSystem().sys();

  // The whole deal here is that we need ghosted versions of vectors y and w (they are parallel, but not ghosted).
  // So to do that I'm going to duplicate current_local_solution (which has the ghosting we want).
  // Then stuff values into the duplicates
  // Then "close()" the vectors which updates their ghosted vaulues.

  {
    // cls is a PetscVector wrapper around the Vec in current_local_solution
    PetscVector<Number> cls(static_cast<PetscVector<Number> *>(system.current_local_solution.get())->vec(), libMesh::CommWorld);

    // Create new NumericVectors with the right ghosting - note: these will be destroyed
    // when this function exits, so nobody better hold pointers to them any more!
    AutoPtr<NumericVector<Number> > ghosted_y_aptr( cls.zero_clone() );
    AutoPtr<NumericVector<Number> > ghosted_w_aptr( cls.zero_clone() );

    // Create PetscVector wrappers around the Vecs.
    PetscVector<Number> ghosted_y( static_cast<PetscVector<Number> *>(ghosted_y_aptr.get())->vec(), libMesh::CommWorld);
    PetscVector<Number> ghosted_w( static_cast<PetscVector<Number> *>(ghosted_w_aptr.get())->vec(), libMesh::CommWorld);

    ierr = VecCopy(y, ghosted_y.vec()); CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = VecCopy(w, ghosted_w.vec()); CHKERRABORT(libMesh::COMM_WORLD,ierr);

    ghosted_y.close();
    ghosted_w.close();

    damping = problem.computeDamping(ghosted_w, ghosted_y);
    if(damping < 1.0)
    {
      //recalculate w=-damping*y + x
      ierr = VecWAXPY(w, -damping, y, x); CHKERRABORT(libMesh::COMM_WORLD,ierr);
      *changed_w = PETSC_TRUE;
    }


    if (problem.shouldUpdateSolution())
    {
      //Update the ghosted copy of w
      if (*changed_w == PETSC_TRUE)
      {
        ierr = VecCopy(w, ghosted_w.vec()); CHKERRABORT(libMesh::COMM_WORLD,ierr);
        ghosted_w.close();
      }

      //Create vector to directly modify w
      PetscVector<Number> vec_w(w, libMesh::CommWorld);

      bool updatedSolution = problem.updateSolution(vec_w, ghosted_w);
      if (updatedSolution)
        *changed_w = PETSC_TRUE;
    }
  }

  return ierr;
}

void petscSetupDampers(NonlinearImplicitSystem& sys)
{
  FEProblem * problem = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
  NonlinearSystem & nl = problem->getNonlinearSystem();
  PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();

#if PETSC_VERSION_LESS_THAN(3,3,0)
  // PETSc 3.2.x-
  SNESLineSearchSetPostCheck(snes, dampedCheck, problem);
#else
  // PETSc 3.3.0+
  SNESLineSearch linesearch;
  PetscErrorCode ierr = SNESGetSNESLineSearch(snes, &linesearch);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  ierr = SNESLineSearchSetPostCheck(linesearch, dampedCheck, problem);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
#endif
}

PCSide
getPetscPCSide(Moose::PCSideType pcs)
{
  switch (pcs)
  {
  case Moose::PCS_LEFT: return PC_LEFT;
  case Moose::PCS_RIGHT: return PC_RIGHT;
  case Moose::PCS_SYMMETRIC: return PC_SYMMETRIC;
  default: mooseError("Unknown PC side requested."); break;
  }
}

void petscSetDefaults(FEProblem & problem)
{
  // dig out Petsc solver
  NonlinearSystem & nl = problem.getNonlinearSystem();
  PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();
  KSP ksp;
  SNESGetKSP(snes, &ksp);
  PCSide pcside = getPetscPCSide(nl.getPCSide());
#if PETSC_VERSION_LESS_THAN(3,2,0)
  // PETSc 3.1.x-
  KSPSetPreconditionerSide(ksp, pcside);
#else
  // PETSc 3.2.x+
  KSPSetPCSide(ksp, pcside);
#endif
  SNESSetMaxLinearSolveFailures(snes, 1000000);

#if PETSC_VERSION_LESS_THAN(3,0,0)
  // PETSc 2.3.3-
  KSPSetConvergenceTest(ksp, petscConverged, &problem);
  SNESSetConvergenceTest(snes, petscNonlinearConverged, &problem);
#else
  // PETSc 3.0.0+

  // In 3.0.0, the context pointer must actually be used, and the
  // final argument to KSPSetConvergenceTest() is a pointer to a
  // routine for destroying said private data context.  In this case,
  // we use the default context provided by PETSc in addition to
  // a few other tests.
  {
    PetscErrorCode ierr = KSPSetConvergenceTest(ksp,
                                                petscConverged,
                                                &problem,
                                                PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = SNESSetConvergenceTest(snes,
                                  petscNonlinearConverged,
                                  &problem,
                                  PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }

#endif
}
} // Namespace PetscSupport
} // Namespace MOOSE

#endif //LIBMESH_HAVE_PETSC
