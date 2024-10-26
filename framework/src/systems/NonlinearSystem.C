//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// moose includes
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "TimeIntegrator.h"
#include "FiniteDifferencePreconditioner.h"
#include "PetscSupport.h"
#include "ComputeResidualFunctor.h"
#include "ComputeFDResidualFunctor.h"
#include "MooseVariableScalar.h"
#include "MooseTypes.h"
#include "SolutionInvalidity.h"
#include "HDGPrimalSolutionUpdateThread.h"
#include "HDGKernel.h"
#include "AuxiliarySystem.h"
#include "Console.h"

#include "libmesh/nonlinear_solver.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/diagonal_matrix.h"
#include "libmesh/default_coupling.h"
#include "libmesh/petsc_solver_exception.h"

using namespace libMesh;

namespace Moose
{
void
compute_jacobian(const NumericVector<Number> & soln,
                 SparseMatrix<Number> & jacobian,
                 NonlinearImplicitSystem & sys)
{
  FEProblemBase * p =
      sys.get_equation_systems().parameters.get<FEProblemBase *>("_fe_problem_base");
  p->computeJacobianSys(sys, soln, jacobian);
}

void
compute_bounds(NumericVector<Number> & lower,
               NumericVector<Number> & upper,
               NonlinearImplicitSystem & sys)
{
  FEProblemBase * p =
      sys.get_equation_systems().parameters.get<FEProblemBase *>("_fe_problem_base");
  p->computeBounds(sys, lower, upper);
}

void
compute_nullspace(std::vector<NumericVector<Number> *> & sp, NonlinearImplicitSystem & sys)
{
  FEProblemBase * p =
      sys.get_equation_systems().parameters.get<FEProblemBase *>("_fe_problem_base");
  p->computeNullSpace(sys, sp);
}

void
compute_transpose_nullspace(std::vector<NumericVector<Number> *> & sp,
                            NonlinearImplicitSystem & sys)
{
  FEProblemBase * p =
      sys.get_equation_systems().parameters.get<FEProblemBase *>("_fe_problem_base");
  p->computeTransposeNullSpace(sys, sp);
}

void
compute_nearnullspace(std::vector<NumericVector<Number> *> & sp, NonlinearImplicitSystem & sys)
{
  FEProblemBase * p =
      sys.get_equation_systems().parameters.get<FEProblemBase *>("_fe_problem_base");
  p->computeNearNullSpace(sys, sp);
}

void
compute_postcheck(const NumericVector<Number> & old_soln,
                  NumericVector<Number> & search_direction,
                  NumericVector<Number> & new_soln,
                  bool & changed_search_direction,
                  bool & changed_new_soln,
                  NonlinearImplicitSystem & sys)
{
  FEProblemBase * p =
      sys.get_equation_systems().parameters.get<FEProblemBase *>("_fe_problem_base");
  p->computePostCheck(
      sys, old_soln, search_direction, new_soln, changed_search_direction, changed_new_soln);
}
} // namespace Moose

NonlinearSystem::NonlinearSystem(FEProblemBase & fe_problem, const std::string & name)
  : NonlinearSystemBase(
        fe_problem, fe_problem.es().add_system<NonlinearImplicitSystem>(name), name),
    _nl_implicit_sys(fe_problem.es().get_system<NonlinearImplicitSystem>(name)),
    _nl_residual_functor(_fe_problem),
    _fd_residual_functor(_fe_problem),
    _resid_and_jac_functor(_fe_problem),
    _use_coloring_finite_difference(false)
{
  nonlinearSolver()->residual_object = &_nl_residual_functor;
  nonlinearSolver()->jacobian = Moose::compute_jacobian;
  nonlinearSolver()->bounds = Moose::compute_bounds;
  nonlinearSolver()->nullspace = Moose::compute_nullspace;
  nonlinearSolver()->transpose_nullspace = Moose::compute_transpose_nullspace;
  nonlinearSolver()->nearnullspace = Moose::compute_nearnullspace;
  nonlinearSolver()->precheck_object = this;

  PetscNonlinearSolver<Real> * petsc_solver =
      static_cast<PetscNonlinearSolver<Real> *>(_nl_implicit_sys.nonlinear_solver.get());
  if (petsc_solver)
  {
    petsc_solver->set_residual_zero_out(false);
    petsc_solver->set_jacobian_zero_out(false);
    petsc_solver->use_default_monitor(false);
  }
}

NonlinearSystem::~NonlinearSystem() {}

void
NonlinearSystem::preInit()
{
  NonlinearSystemBase::preInit();

  if (_automatic_scaling && _resid_vs_jac_scaling_param < 1. - TOLERANCE)
    // Add diagonal matrix that will be used for computing scaling factors
    _nl_implicit_sys.add_matrix<DiagonalMatrix>("scaling_matrix");

  if (_hybridized_kernels.hasObjects())
    addVector(HDGKernel::lm_increment_vector_name, true, GHOSTED);
}

void
NonlinearSystem::potentiallySetupFiniteDifferencing()
{
  if (_use_finite_differenced_preconditioner)
  {
    _nl_implicit_sys.nonlinear_solver->fd_residual_object = &_fd_residual_functor;
    setupFiniteDifferencedPreconditioner();
  }

  PetscNonlinearSolver<Real> & solver =
      static_cast<PetscNonlinearSolver<Real> &>(*_nl_implicit_sys.nonlinear_solver);
  solver.mffd_residual_object = &_fd_residual_functor;

  solver.set_snesmf_reuse_base(_fe_problem.useSNESMFReuseBase());
}

void
NonlinearSystem::solve()
{
  // Only attach the postcheck function to the solver if we actually
  // have dampers or if the FEProblemBase needs to update the solution,
  // which is also done during the linesearch postcheck.  It doesn't
  // hurt to do this multiple times, it is just setting a pointer.
  if (_fe_problem.hasDampers() || _fe_problem.shouldUpdateSolution() ||
      _fe_problem.needsPreviousNewtonIteration())
    _nl_implicit_sys.nonlinear_solver->postcheck = Moose::compute_postcheck;

  // reset solution invalid counter for the time step
  if (!_time_integrators.empty())
    _app.solutionInvalidity().resetSolutionInvalidTimeStep();

  if (shouldEvaluatePreSMOResidual())
  {
    TIME_SECTION("nlPreSMOResidual", 3, "Computing Pre-SMO Residual");
    // Calculate the pre-SMO residual for use in the convergence criterion.
    _computing_pre_smo_residual = true;
    _fe_problem.computeResidualSys(_nl_implicit_sys, *_current_solution, *_nl_implicit_sys.rhs);
    _computing_pre_smo_residual = false;
    _nl_implicit_sys.rhs->close();
    _pre_smo_residual = _nl_implicit_sys.rhs->l2_norm();
    _console << " * Nonlinear |R| = "
             << Console::outputNorm(std::numeric_limits<Real>::max(), _pre_smo_residual)
             << " (Before preset BCs, predictors, correctors, and constraints)\n";
    _console << std::flush;
  }

  const bool presolve_succeeded = preSolve();
  if (!presolve_succeeded)
    return;

  potentiallySetupFiniteDifferencing();

  const bool time_integrator_solve = std::any_of(_time_integrators.begin(),
                                                 _time_integrators.end(),
                                                 [](auto & ti) { return ti->overridesSolve(); });
  if (time_integrator_solve)
    mooseAssert(_time_integrators.size() == 1,
                "If solve is overridden, then there must be only one time integrator");

  if (time_integrator_solve)
    _time_integrators.front()->solve();
  else
    system().solve();

  for (auto & ti : _time_integrators)
  {
    if (!ti->overridesSolve())
      ti->setNumIterationsLastSolve();
    ti->postSolve();
  }

  if (!_time_integrators.empty())
  {
    _n_iters = _time_integrators.front()->getNumNonlinearIterations();
    _n_linear_iters = _time_integrators.front()->getNumLinearIterations();
  }
  else
  {
    _n_iters = _nl_implicit_sys.n_nonlinear_iterations();
    _n_linear_iters = _nl_implicit_sys.nonlinear_solver->get_total_linear_iterations();
  }

  // store info about the solve
  _final_residual = _nl_implicit_sys.final_nonlinear_residual();

  // Accumulate only the occurence of solution invalid warnings
  _app.solutionInvalidity().solutionInvalidAccumulationTimeStep();

  // determine whether solution invalid occurs in the converged solution
  checkInvalidSolution();

  if (_use_coloring_finite_difference)
    LibmeshPetscCall(MatFDColoringDestroy(&_fdcoloring));
}

void
NonlinearSystem::stopSolve(const ExecFlagType & exec_flag,
                           const std::set<TagID> & vector_tags_to_close)
{
  PetscNonlinearSolver<Real> & solver =
      static_cast<PetscNonlinearSolver<Real> &>(*sys().nonlinear_solver);

  if (exec_flag == EXEC_LINEAR || exec_flag == EXEC_POSTCHECK)
  {
    LibmeshPetscCall(SNESSetFunctionDomainError(solver.snes()));

    // Clean up by getting vectors into a valid state for a
    // (possible) subsequent solve.
    closeTaggedVectors(vector_tags_to_close);
  }
  else if (exec_flag == EXEC_NONLINEAR)
    LibmeshPetscCall(SNESSetJacobianDomainError(solver.snes()));
  else
    mooseError("Unsupported execute flag: ", Moose::stringify(exec_flag));
}

void
NonlinearSystem::setupFiniteDifferencedPreconditioner()
{
  std::shared_ptr<FiniteDifferencePreconditioner> fdp =
      std::dynamic_pointer_cast<FiniteDifferencePreconditioner>(_preconditioner);
  if (!fdp)
    mooseError("Did not setup finite difference preconditioner, and please add a preconditioning "
               "block with type = fdp");

  if (fdp->finiteDifferenceType() == "coloring")
  {
    _use_coloring_finite_difference = true;
    setupColoringFiniteDifferencedPreconditioner();
  }

  else if (fdp->finiteDifferenceType() == "standard")
  {
    _use_coloring_finite_difference = false;
    setupStandardFiniteDifferencedPreconditioner();
  }
  else
    mooseError("Unknown finite difference type");
}

void
NonlinearSystem::setupStandardFiniteDifferencedPreconditioner()
{
  // Make sure that libMesh isn't going to override our preconditioner
  _nl_implicit_sys.nonlinear_solver->jacobian = nullptr;

  PetscNonlinearSolver<Number> * petsc_nonlinear_solver =
      static_cast<PetscNonlinearSolver<Number> *>(_nl_implicit_sys.nonlinear_solver.get());

  PetscMatrix<Number> * petsc_mat =
      static_cast<PetscMatrix<Number> *>(&_nl_implicit_sys.get_system_matrix());

  LibmeshPetscCall(SNESSetJacobian(petsc_nonlinear_solver->snes(),
                                   petsc_mat->mat(),
                                   petsc_mat->mat(),
                                   SNESComputeJacobianDefault,
                                   nullptr));
}

void
NonlinearSystem::setupColoringFiniteDifferencedPreconditioner()
{
  // Make sure that libMesh isn't going to override our preconditioner
  _nl_implicit_sys.nonlinear_solver->jacobian = nullptr;

  PetscNonlinearSolver<Number> & petsc_nonlinear_solver =
      dynamic_cast<PetscNonlinearSolver<Number> &>(*_nl_implicit_sys.nonlinear_solver);

  // Pointer to underlying PetscMatrix type
  PetscMatrix<Number> * petsc_mat =
      dynamic_cast<PetscMatrix<Number> *>(&_nl_implicit_sys.get_system_matrix());

  Moose::compute_jacobian(*_nl_implicit_sys.current_local_solution, *petsc_mat, _nl_implicit_sys);

  if (!petsc_mat)
    mooseError("Could not convert to Petsc matrix.");

  petsc_mat->close();

  ISColoring iscoloring;

  // PETSc 3.5.x
  MatColoring matcoloring;
  LibmeshPetscCallA(_communicator.get(), MatColoringCreate(petsc_mat->mat(), &matcoloring));
  LibmeshPetscCallA(_communicator.get(), MatColoringSetType(matcoloring, MATCOLORINGLF));
  LibmeshPetscCallA(_communicator.get(), MatColoringSetFromOptions(matcoloring));
  LibmeshPetscCallA(_communicator.get(), MatColoringApply(matcoloring, &iscoloring));
  LibmeshPetscCallA(_communicator.get(), MatColoringDestroy(&matcoloring));

  LibmeshPetscCallA(_communicator.get(),
                    MatFDColoringCreate(petsc_mat->mat(), iscoloring, &_fdcoloring));
  LibmeshPetscCallA(_communicator.get(), MatFDColoringSetFromOptions(_fdcoloring));
  // clang-format off
  LibmeshPetscCallA(_communicator.get(), MatFDColoringSetFunction(_fdcoloring,
                                                                  (PetscErrorCode(*)(void))(void (*)(void)) &
                                                                      libMesh::libmesh_petsc_snes_fd_residual,
                                                                  &petsc_nonlinear_solver));
  // clang-format on
  LibmeshPetscCallA(_communicator.get(),
                    MatFDColoringSetUp(petsc_mat->mat(), iscoloring, _fdcoloring));
  LibmeshPetscCallA(_communicator.get(),
                    SNESSetJacobian(petsc_nonlinear_solver.snes(),
                                    petsc_mat->mat(),
                                    petsc_mat->mat(),
                                    SNESComputeJacobianDefaultColor,
                                    _fdcoloring));
  // PETSc >=3.3.0
  LibmeshPetscCallA(_communicator.get(), ISColoringDestroy(&iscoloring));
}

bool
NonlinearSystem::converged()
{
  if (_fe_problem.hasException() || _fe_problem.getFailNextNonlinearConvergenceCheck())
    return false;
  if (!_fe_problem.acceptInvalidSolution())
  {
    mooseWarning("The solution is not converged due to the solution being invalid.");
    return false;
  }
  return _nl_implicit_sys.nonlinear_solver->converged;
}

void
NonlinearSystem::attachPreconditioner(Preconditioner<Number> * preconditioner)
{
  nonlinearSolver()->attach_preconditioner(preconditioner);
}

void
NonlinearSystem::computeScalingJacobian()
{
  _fe_problem.computeJacobianSys(_nl_implicit_sys, *_current_solution, *_scaling_matrix);
}

void
NonlinearSystem::computeScalingResidual()
{
  _fe_problem.computeResidualSys(_nl_implicit_sys, *_current_solution, RHS());
}

SNES
NonlinearSystem::getSNES()
{
  PetscNonlinearSolver<Number> * petsc_solver =
      dynamic_cast<PetscNonlinearSolver<Number> *>(nonlinearSolver());

  if (petsc_solver)
    return petsc_solver->snes();
  else
    mooseError("It is not a petsc nonlinear solver");
}

void
NonlinearSystem::residualAndJacobianTogether()
{
  if (_fe_problem.solverParams()._type == Moose::ST_JFNK)
    mooseError(
        "Evaluting the residual and Jacobian together does not make sense for a JFNK solve type in "
        "which only function evaluations are required, e.g. there is no need to form a matrix");

  nonlinearSolver()->residual_object = nullptr;
  nonlinearSolver()->jacobian = nullptr;
  nonlinearSolver()->residual_and_jacobian_object = &_resid_and_jac_functor;
}

void
NonlinearSystem::precheck(const NumericVector<Number> & /*precheck_soln*/,
                          NumericVector<Number> & search_direction,
                          bool & /*changed*/,
                          NonlinearImplicitSystem & /*S*/)
{
  if (!_hybridized_kernels.hasActiveObjects())
    return;

  auto & ghosted_increment = getVector(HDGKernel::lm_increment_vector_name);
  ghosted_increment.zero();
  // The search direction coming from PETSc is the negative of the solution update
  ghosted_increment -= search_direction;

  PARALLEL_TRY
  {
    TIME_SECTION("HDG kernel primal solution update",
                 3 /*, "Computing hybridized kernel primal solution update"*/);
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    HDGPrimalSolutionUpdateThread pre_thread(_fe_problem, _hybridized_kernels);
    Threads::parallel_reduce(elem_range, pre_thread);
  }
  PARALLEL_CATCH;
  // The primal variables live in the aux system
  auto & aux = _fe_problem.getAuxiliarySystem();
  aux.solution().close();
  // scatter into ghosted current local solution
  aux.update();
}
