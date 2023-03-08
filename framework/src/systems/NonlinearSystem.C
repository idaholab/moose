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

#include "libmesh/nonlinear_solver.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/diagonal_matrix.h"
#include "libmesh/default_coupling.h"

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
    _use_coloring_finite_difference(false),
    _solution_is_invalid(false)
{
  nonlinearSolver()->residual_object = &_nl_residual_functor;
  nonlinearSolver()->jacobian = Moose::compute_jacobian;
  nonlinearSolver()->bounds = Moose::compute_bounds;
  nonlinearSolver()->nullspace = Moose::compute_nullspace;
  nonlinearSolver()->transpose_nullspace = Moose::compute_transpose_nullspace;
  nonlinearSolver()->nearnullspace = Moose::compute_nearnullspace;

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
NonlinearSystem::init()
{
  NonlinearSystemBase::init();

  if (_automatic_scaling && _resid_vs_jac_scaling_param < 1. - TOLERANCE)
    // Add diagonal matrix that will be used for computing scaling factors
    _nl_implicit_sys.add_matrix<DiagonalMatrix>("scaling_matrix");
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

  if (_fe_problem.solverParams()._type != Moose::ST_LINEAR)
  {
    TIME_SECTION("nlInitialResidual", 3, "Computing Initial Residual");
    // Calculate the initial residual for use in the convergence criterion.
    _computing_initial_residual = true;
    _fe_problem.computeResidualSys(_nl_implicit_sys, *_current_solution, *_nl_implicit_sys.rhs);
    _computing_initial_residual = false;
    _nl_implicit_sys.rhs->close();
    _initial_residual_before_preset_bcs = _nl_implicit_sys.rhs->l2_norm();
    if (_compute_initial_residual_before_preset_bcs)
      _console << "Initial residual before setting preset BCs: "
               << _initial_residual_before_preset_bcs << std::endl;
  }

  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;

  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  // Now that the initial solution has ben set, potentially perform a residual/Jacobian evaluation
  // to determine variable scaling factors
  if (_automatic_scaling)
  {
    if (_compute_scaling_once)
    {
      if (!_computed_scaling)
      {
        computeScaling();
        _computed_scaling = true;
      }
    }
    else
      computeScaling();
  }
  // We do not know a priori what variable a global degree of freedom corresponds to, so we need a
  // map from global dof to scaling factor. We just use a ghosted NumericVector for that mapping
  assembleScalingVector();

  if (_use_finite_differenced_preconditioner)
  {
    _nl_implicit_sys.nonlinear_solver->fd_residual_object = &_fd_residual_functor;
    setupFiniteDifferencedPreconditioner();
  }

  PetscNonlinearSolver<Real> & solver =
      static_cast<PetscNonlinearSolver<Real> &>(*_nl_implicit_sys.nonlinear_solver);
  solver.mffd_residual_object = &_fd_residual_functor;

  solver.set_snesmf_reuse_base(_fe_problem.useSNESMFReuseBase());

  if (_time_integrator)
  {
    // reset solution invalid counter for the time step
    _app.solutionInvalidity().resetSolutionInvalidTimeStep();
    _time_integrator->solve();
    _time_integrator->postSolve();
    _n_iters = _time_integrator->getNumNonlinearIterations();
    _n_linear_iters = _time_integrator->getNumLinearIterations();
    // Accumulate only the occurence of solution invalid warnings for the current time step counters
    _app.solutionInvalidity().solutionInvalidAccumulationTimeStep();
  }
  else
  {
    system().solve();
    _n_iters = _nl_implicit_sys.n_nonlinear_iterations();
    _n_linear_iters = solver.get_total_linear_iterations();
  }

  // store info about the solve
  _final_residual = _nl_implicit_sys.final_nonlinear_residual();

  // determine whether solution invalid occurs in the converged solution
  _solution_is_invalid = _app.solutionInvalidity().solutionInvalid();

  // output the solution invalid summary
  if (_solution_is_invalid)
  {
    // sync all solution invalid counts to rank 0 process
    _app.solutionInvalidity().sync();

    if (_fe_problem.allowInvalidSolution())
      mooseWarning("The Solution Invalidity warnings are detected but silenced! "
                   "Use Problem/allow_invalid_solution=false to activate ");
    else
      // output the occurrence of solution invalid in a summary table
      _app.solutionInvalidity().print(_console);
  }

  if (_use_coloring_finite_difference)
    MatFDColoringDestroy(&_fdcoloring);
}

void
NonlinearSystem::stopSolve()
{
  PetscNonlinearSolver<Real> & solver =
      static_cast<PetscNonlinearSolver<Real> &>(*sys().nonlinear_solver);
  SNESSetFunctionDomainError(solver.snes());

  // Insert a NaN into the residual vector.  As of PETSc-3.6, this
  // should make PETSc return DIVERGED_NANORINF the next time it does
  // a reduction.  We'll write to the first local dof on every
  // processor that has any dofs.
  _nl_implicit_sys.rhs->close();

  if (_nl_implicit_sys.rhs->local_size())
    _nl_implicit_sys.rhs->set(_nl_implicit_sys.rhs->first_local_index(),
                              std::numeric_limits<Real>::quiet_NaN());
  _nl_implicit_sys.rhs->close();

  // Clean up by getting other vectors into a valid state for a
  // (possible) subsequent solve.  There may be more than just
  // these...
  if (_Re_time)
    _Re_time->close();
  _Re_non_time->close();
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
    setupColoringFiniteDifferencedPreconditioner();
    _use_coloring_finite_difference = true;
  }

  else if (fdp->finiteDifferenceType() == "standard")
  {
    setupStandardFiniteDifferencedPreconditioner();
    _use_coloring_finite_difference = false;
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

  SNESSetJacobian(petsc_nonlinear_solver->snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESComputeJacobianDefault,
                  nullptr);
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

  PetscErrorCode ierr = 0;
  ISColoring iscoloring;

  // PETSc 3.5.x
  MatColoring matcoloring;
  ierr = MatColoringCreate(petsc_mat->mat(), &matcoloring);
  CHKERRABORT(_communicator.get(), ierr);
  ierr = MatColoringSetType(matcoloring, MATCOLORINGLF);
  CHKERRABORT(_communicator.get(), ierr);
  ierr = MatColoringSetFromOptions(matcoloring);
  CHKERRABORT(_communicator.get(), ierr);
  ierr = MatColoringApply(matcoloring, &iscoloring);
  CHKERRABORT(_communicator.get(), ierr);
  ierr = MatColoringDestroy(&matcoloring);
  CHKERRABORT(_communicator.get(), ierr);

  MatFDColoringCreate(petsc_mat->mat(), iscoloring, &_fdcoloring);
  MatFDColoringSetFromOptions(_fdcoloring);
  // clang-format off
  MatFDColoringSetFunction(_fdcoloring,
                           (PetscErrorCode(*)(void))(void (*)(void)) &
                               libMesh::libmesh_petsc_snes_fd_residual,
                           &petsc_nonlinear_solver);
  // clang-format on
  MatFDColoringSetUp(petsc_mat->mat(), iscoloring, _fdcoloring);
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESComputeJacobianDefaultColor,
                  _fdcoloring);
  // PETSc >=3.3.0
  ISColoringDestroy(&iscoloring);
}

bool
NonlinearSystem::converged()
{
  if (_fe_problem.hasException())
    return false;
  if (!_fe_problem.allowInvalidSolution() && _solution_is_invalid)
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
