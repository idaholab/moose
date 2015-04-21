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

// Moose includes
#include "MooseApp.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "NonlinearSystem.h"
#include "DisplacedProblem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "MooseTypes.h"
#include "MooseUtils.h"
#include "CommandLine.h"
#include "Console.h"
#include "MultiMooseEnum.h"

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
#include <petscdm.h>
#endif

//PetscDMMoose include
#include "PetscDMMoose.h"

// Standard includes
#include <ostream>
#include <fstream>
#include <string>

namespace Moose
{
namespace PetscSupport
{

std::string
stringify(const LineSearchType & t)
{
  switch (t)
  {
  case LS_BASIC:   return "basic";
  case LS_DEFAULT: return "default";
  case LS_NONE:    return "none";
#if PETSC_VERSION_LESS_THAN(3,3,0)
  case LS_CUBIC:        return "cubic";
  case LS_QUADRATIC:    return "quadratic";
  case LS_BASICNONORMS: return "basicnonorms";
#else
  case LS_SHELL: return "shell";
  case LS_L2:    return "l2";
  case LS_BT:    return "bt";
  case LS_CP:    return "cp";
#endif
  case LS_INVALID: mooseError("Invalid LineSearchType");
  }
  return "";
}

void
setSolverOptions(SolverParams & solver_params)
{
  // set PETSc options implied by a solve type
  switch (solver_params._type)
  {
  case Moose::ST_PJFNK:
    PetscOptionsSetValue("-snes_mf_operator", PETSC_NULL);
    break;

  case Moose::ST_JFNK:
    PetscOptionsSetValue("-snes_mf", PETSC_NULL);
    break;

  case Moose::ST_NEWTON:
    break;

  case Moose::ST_FD:
    PetscOptionsSetValue("-snes_fd", PETSC_NULL);
    break;

  case Moose::ST_LINEAR:
    PetscOptionsSetValue("-snes_type", "ksponly");
    break;
  }

  Moose::LineSearchType ls_type = solver_params._line_search;
  if (ls_type == Moose::LS_NONE)
    ls_type = Moose::LS_BASIC;

  if (ls_type != Moose::LS_DEFAULT)
  {
#if PETSC_VERSION_LESS_THAN(3,3,0)
    PetscOptionsSetValue("-snes_type", "ls");
    PetscOptionsSetValue("-snes_ls", stringify(ls_type));
#else
    PetscOptionsSetValue("-snes_linesearch_type", stringify(ls_type).c_str());
#endif
  }
}

void petscSetupDM (NonlinearSystem & nl) {
#if !PETSC_VERSION_LESS_THAN(3,3,0)
  PetscErrorCode  ierr;

  // Initialize the part of the DM package that's packaged with Moose; in the PETSc source tree this call would be in DMInitializePackage()
  ierr = DMMooseRegisterAll();
  CHKERRABORT(nl.comm().get(),ierr);
  // Create and set up the DM that will consume the split options and deal with block matrices.
  PetscNonlinearSolver<Number> *petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();
  /* FIXME: reset the DM, do not recreate it anew every time? */
  DM dm = PETSC_NULL;
  ierr = DMCreateMoose(nl.comm().get(), nl, &dm);
  CHKERRABORT(nl.comm().get(),ierr);
  ierr = DMSetFromOptions(dm);
  CHKERRABORT(nl.comm().get(),ierr);
  ierr = DMSetUp(dm);
  CHKERRABORT(nl.comm().get(),ierr);
  ierr = SNESSetDM(snes,dm);
  CHKERRABORT(nl.comm().get(),ierr);
  ierr = DMDestroy(&dm);
  CHKERRABORT(nl.comm().get(),ierr);
  ierr = SNESSetUpdate(snes,SNESUpdateDMMoose);
  CHKERRABORT(nl.comm().get(),ierr);
#endif
}


void
petscSetOptions(FEProblem & problem)
{
        MultiMooseEnum             petsc_options = problem.parameters().get<MultiMooseEnum>("petsc_options");
  const std::vector<std::string> & petsc_options_inames = problem.parameters().get<std::vector<std::string> >("petsc_inames");
  const std::vector<std::string> & petsc_options_values = problem.parameters().get<std::vector<std::string> >("petsc_values");

  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("PETSc names and options are not the same length");

  PetscOptionsClear();

  { // Get any options specified on the command-line
    int argc;
    char ** args;

    PetscGetArgs(&argc, &args);
    PetscOptionsInsert(&argc, &args, NULL);
  }

  setSolverOptions(problem.solverParams());

  // Add any additional options specified in the input file
  for (MooseEnumIterator it = petsc_options.begin(); it != petsc_options.end(); ++it)
    PetscOptionsSetValue(it->c_str(), PETSC_NULL);
  for (unsigned int i=0; i<petsc_options_inames.size(); ++i)
    PetscOptionsSetValue(petsc_options_inames[i].c_str(), petsc_options_values[i].c_str());

  SolverParams& solver_params = problem.solverParams();
  if (solver_params._type != Moose::ST_JFNK  &&
      solver_params._type != Moose::ST_FD &&
      !problem.getNonlinearSystem().haveFiniteDifferencedPreconditioner() &&
      problem.getNonlinearSystem().haveDecomposition())
  {
    // Set up DM only if have a decomposition. Additionally, turn DM OFF if not using FD-based solvers,
    // (both -snes_mf and -snes_fd) and FDP. This is all rather crufty, but what's a good generic rule here?
    // In principle at least, splits should be able to work with ST_FD (-snes_fd) and FDP (a coloring-based
    // version of -snes_fd), but one has to be careful about the initialization order so as not to override
    // SNESComputeJacobianDefaultColor() set up by FDP, for instance.  However, it's unlikely that splits
    // will be used when running an FD solver (debugging).
    problem.getNonlinearSystem().setupDecomposition();
    petscSetupDM(problem.getNonlinearSystem());
  } else {
    // Otherwise turn off the decomposition
    std::vector<std::string> nosplits;
    problem.getNonlinearSystem().setDecomposition(nosplits);
  }

}

PetscErrorCode petscSetupOutput(CommandLine * cmd_line)
{
  char code[10] = {45,45,109,111,111,115,101};
  if (cmd_line->getPot()->search(code))
    Console::petscSetupOutput();
  return 0;
}

PetscErrorCode petscConverged(KSP ksp, PetscInt n, PetscReal rnorm, KSPConvergedReason * reason, void * ctx)
{
  // Cast the context pointer coming from PETSc to an FEProblem& and
  // get a reference to the System from it.
  FEProblem & problem = *static_cast<FEProblem *>(ctx);

  // Let's be nice and always check PETSc error codes.
  PetscErrorCode ierr = 0;

  // We want the default behavior of the KSPDefaultConverged test, but
  // we don't want PETSc to die in that function with a CHKERRQ
  // call... that is probably extremely unlikely/impossible, but just
  // to be on the safe side, we push a different error handler before
  // calling KSPDefaultConverged().
  ierr = PetscPushErrorHandler(PetscReturnErrorHandler, /*void* ctx=*/ PETSC_NULL);
  CHKERRABORT(problem.comm().get(),ierr);

#if PETSC_VERSION_LESS_THAN(3,0,0)
  // Prior to PETSc 3.0.0, you could call KSPDefaultConverged with a NULL context
  // pointer, as it was unused.
  KSPDefaultConverged(ksp, n, rnorm, reason, PETSC_NULL);
#elif PETSC_RELEASE_LESS_THAN(3,5,0)
  // As of PETSc 3.0.0, you must call KSPDefaultConverged with a
  // non-NULL context pointer which must be created with
  // KSPDefaultConvergedCreate(), and destroyed with
  // KSPDefaultConvergedDestroy().
  void* default_ctx = NULL;
  KSPDefaultConvergedCreate(&default_ctx);
  KSPDefaultConverged(ksp, n, rnorm, reason, default_ctx);
  KSPDefaultConvergedDestroy(default_ctx);
#else
  // As of PETSc 3.5.0, use KSPConvergedDefaultXXX
  void* default_ctx = NULL;
  KSPConvergedDefaultCreate(&default_ctx);
  KSPConvergedDefault(ksp, n, rnorm, reason, default_ctx);
  KSPConvergedDefaultDestroy(default_ctx);
#endif

  // Pop the Error handler we pushed on the stack to go back
  // to default PETSc error handling behavior.
  ierr = PetscPopErrorHandler();
  CHKERRABORT(problem.comm().get(),ierr);

  // Get tolerances from the KSP object
  PetscReal rtol = 0.;
  PetscReal atol = 0.;
  PetscReal dtol = 0.;
  PetscInt maxits = 0;
  ierr = KSPGetTolerances(ksp, &rtol, &atol, &dtol, &maxits);
  CHKERRABORT(problem.comm().get(),ierr);

  // Now do some additional MOOSE-specific tests...
  std::string msg;
  MooseLinearConvergenceReason moose_reason = problem.checkLinearConvergence(msg, n, rnorm, rtol, atol, dtol, maxits);

  switch (moose_reason)
  {
  case MOOSE_CONVERGED_RTOL:
    *reason = KSP_CONVERGED_RTOL;
    break;

  case MOOSE_CONVERGED_ITS:
    *reason = KSP_CONVERGED_ITS;
    break;

  default:
  {
    // If it's not either of the two specific cases we handle, just go
    // with whatever PETSc decided in KSPDefaultConverged.
    break;
  }
  }

  return 0;
}

PetscErrorCode petscNonlinearConverged(SNES snes, PetscInt it, PetscReal xnorm, PetscReal snorm, PetscReal fnorm, SNESConvergedReason * reason, void * ctx)
{
  FEProblem & problem = *static_cast<FEProblem *>(ctx);
  NonlinearSystem & system = problem.getNonlinearSystem();

  // Let's be nice and always check PETSc error codes.
  PetscErrorCode ierr = 0;

  // Temporary variables to store SNES tolerances.  Usual C-style would be to declare
  // but not initialize these... but it bothers me to leave anything uninitialized.
  PetscReal atol = 0.; // absolute convergence tolerance
  PetscReal rtol = 0.; // relative convergence tolerance
  PetscReal stol = 0.; // convergence (step) tolerance in terms of the norm of the change in the solution between steps
  PetscInt maxit = 0;  // maximum number of iterations
  PetscInt maxf = 0;   // maximum number of function evaluations

  // Ask the SNES object about its tolerances.
  ierr = SNESGetTolerances(snes,
                           &atol,
                           &rtol,
                           &stol,
                           &maxit,
                           &maxf);
  CHKERRABORT(problem.comm().get(),ierr);

  // Get current number of function evaluations done by SNES.
  PetscInt nfuncs = 0;
  ierr = SNESGetNumberFunctionEvals(snes, &nfuncs);
  CHKERRABORT(problem.comm().get(),ierr);

  // Error message that will be set by the FEProblem.
  std::string msg;

  // MOOSE-specific parameters
  const Real ref_resid = system._initial_residual;
  const Real div_threshold = system._initial_residual*(1.0/rtol);

  // xnorm: 2-norm of current iterate
  // snorm: 2-norm of current step
  // fnorm: 2-norm of function at current iterate
  MooseNonlinearConvergenceReason moose_reason = problem.checkNonlinearConvergence(msg,
                                                                                   it,
                                                                                   xnorm,
                                                                                   snorm,
                                                                                   fnorm,
                                                                                   rtol,
                                                                                   stol,
                                                                                   atol,
                                                                                   nfuncs,
                                                                                   maxf,
                                                                                   ref_resid,
                                                                                   div_threshold);

  if (msg.length() > 0)
    PetscInfo(snes, msg.c_str());

  switch (moose_reason)
  {
    case MOOSE_NONLINEAR_ITERATING:
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

  return 0;
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
    PetscVector<Number> cls(static_cast<PetscVector<Number> *>(system.current_local_solution.get())->vec(), problem.comm());

    // Create new NumericVectors with the right ghosting - note: these will be destroyed
    // when this function exits, so nobody better hold pointers to them any more!
    UniquePtr<NumericVector<Number> > ghosted_y_aptr( cls.zero_clone() );
    UniquePtr<NumericVector<Number> > ghosted_w_aptr( cls.zero_clone() );

    // Create PetscVector wrappers around the Vecs.
    PetscVector<Number> ghosted_y( static_cast<PetscVector<Number> *>(ghosted_y_aptr.get())->vec(), problem.comm());
    PetscVector<Number> ghosted_w( static_cast<PetscVector<Number> *>(ghosted_w_aptr.get())->vec(), problem.comm());

    ierr = VecCopy(y, ghosted_y.vec()); CHKERRABORT(problem.comm().get(),ierr);
    ierr = VecCopy(w, ghosted_w.vec()); CHKERRABORT(problem.comm().get(),ierr);

    ghosted_y.close();
    ghosted_w.close();

    damping = problem.computeDamping(ghosted_w, ghosted_y);
    if (damping < 1.0)
    {
      //recalculate w=-damping*y + x
      ierr = VecWAXPY(w, -damping, y, x); CHKERRABORT(problem.comm().get(),ierr);
      *changed_w = PETSC_TRUE;
    }


    if (problem.shouldUpdateSolution())
    {
      //Update the ghosted copy of w
      if (*changed_w == PETSC_TRUE)
      {
        ierr = VecCopy(w, ghosted_w.vec()); CHKERRABORT(problem.comm().get(),ierr);
        ghosted_w.close();
      }

      //Create vector to directly modify w
      PetscVector<Number> vec_w(w, problem.comm());

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
#if PETSC_VERSION_LESS_THAN(3,4,0)
  PetscErrorCode ierr = SNESGetSNESLineSearch(snes, &linesearch);
#else
  PetscErrorCode ierr = SNESGetLineSearch(snes, &linesearch);
#endif
  CHKERRABORT(problem->comm().get(),ierr);

  ierr = SNESLineSearchSetPostCheck(linesearch, dampedCheck, problem);
  CHKERRABORT(problem->comm().get(),ierr);
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
    CHKERRABORT(nl.comm().get(),ierr);
    ierr = SNESSetConvergenceTest(snes,
                                  petscNonlinearConverged,
                                  &problem,
                                  PETSC_NULL);
    CHKERRABORT(nl.comm().get(),ierr);
  }
#endif
}
} // Namespace PetscSupport
} // Namespace MOOSE

#endif //LIBMESH_HAVE_PETSC
