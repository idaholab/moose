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
#include "TimeIntegrator.h"
#include "FiniteDifferencePreconditioner.h"
#include "PetscSupport.h"
#include "ComputeResidualFunctor.h"
#include "ComputeFDResidualFunctor.h"

#include "libmesh/nonlinear_solver.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"

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
        fe_problem, fe_problem.es().add_system<TransientNonlinearImplicitSystem>(name), name),
    _transient_sys(fe_problem.es().get_system<TransientNonlinearImplicitSystem>(name)),
    _nl_residual_functor(_fe_problem),
    _fd_residual_functor(_fe_problem),
    _use_coloring_finite_difference(false)
{
  nonlinearSolver()->residual_object = &_nl_residual_functor;
  nonlinearSolver()->jacobian = Moose::compute_jacobian;
  nonlinearSolver()->bounds = Moose::compute_bounds;
  nonlinearSolver()->nullspace = Moose::compute_nullspace;
  nonlinearSolver()->transpose_nullspace = Moose::compute_transpose_nullspace;
  nonlinearSolver()->nearnullspace = Moose::compute_nearnullspace;

#ifdef LIBMESH_HAVE_PETSC
  PetscNonlinearSolver<Real> * petsc_solver =
      static_cast<PetscNonlinearSolver<Real> *>(_transient_sys.nonlinear_solver.get());
  if (petsc_solver)
  {
    petsc_solver->set_residual_zero_out(false);
    petsc_solver->set_jacobian_zero_out(false);
    petsc_solver->use_default_monitor(false);
  }
#endif
}

NonlinearSystem::~NonlinearSystem() {}

SparseMatrix<Number> &
NonlinearSystem::addMatrix(TagID tag)
{
  if (!_subproblem.matrixTagExists(tag))
    mooseError("Cannot add a tagged matrix with matrix_tag, ",
               tag,
               ", that tag does not exist in System ",
               name());

  if (hasMatrix(tag))
    return getMatrix(tag);

  auto matrix_name = _subproblem.matrixTagName(tag);

  SparseMatrix<Number> * mat = &_transient_sys.add_matrix(matrix_name);

  if (_tagged_matrices.size() < tag + 1)
    _tagged_matrices.resize(tag + 1);

  _tagged_matrices[tag] = mat;

  return *mat;
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
    _transient_sys.nonlinear_solver->postcheck = Moose::compute_postcheck;

  if (_fe_problem.solverParams()._type != Moose::ST_LINEAR)
  {
    // Calculate the initial residual for use in the convergence criterion.
    _computing_initial_residual = true;
    _fe_problem.computeResidualSys(_transient_sys, *_current_solution, *_transient_sys.rhs);
    _computing_initial_residual = false;
    _transient_sys.rhs->close();
    _initial_residual_before_preset_bcs = _transient_sys.rhs->l2_norm();
    if (_compute_initial_residual_before_preset_bcs)
      _console << "Initial residual before setting preset BCs: "
               << _initial_residual_before_preset_bcs << '\n';
  }

  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;

  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  if (_use_finite_differenced_preconditioner)
  {
    _transient_sys.nonlinear_solver->fd_residual_object = &_fd_residual_functor;
    setupFiniteDifferencedPreconditioner();
  }

#ifdef LIBMESH_HAVE_PETSC
  PetscNonlinearSolver<Real> & solver =
      static_cast<PetscNonlinearSolver<Real> &>(*_transient_sys.nonlinear_solver);
  solver.mffd_residual_object = &_fd_residual_functor;

  solver.set_snesmf_reuse_base(_fe_problem.useSNESMFReuseBase());
#endif

  if (_time_integrator)
  {
    _time_integrator->solve();
    _time_integrator->postSolve();
    _n_iters = _time_integrator->getNumNonlinearIterations();
    _n_linear_iters = _time_integrator->getNumLinearIterations();
  }
  else
  {
    system().solve();
    _n_iters = _transient_sys.n_nonlinear_iterations();
#ifdef LIBMESH_HAVE_PETSC
    _n_linear_iters = solver.get_total_linear_iterations();
#endif
  }

  // store info about the solve
  _final_residual = _transient_sys.final_nonlinear_residual();

#ifdef LIBMESH_HAVE_PETSC
  if (_use_coloring_finite_difference)
#if PETSC_VERSION_LESS_THAN(3, 2, 0)
    MatFDColoringDestroy(_fdcoloring);
#else
    MatFDColoringDestroy(&_fdcoloring);
#endif
#endif
}

void
NonlinearSystem::stopSolve()
{
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3, 0, 0)
#else
  PetscNonlinearSolver<Real> & solver =
      static_cast<PetscNonlinearSolver<Real> &>(*sys().nonlinear_solver);
  SNESSetFunctionDomainError(solver.snes());
#endif
#endif

  // Insert a NaN into the residual vector.  As of PETSc-3.6, this
  // should make PETSc return DIVERGED_NANORINF the next time it does
  // a reduction.  We'll write to the first local dof on every
  // processor that has any dofs.
  _transient_sys.rhs->close();

  if (_transient_sys.rhs->local_size())
    _transient_sys.rhs->set(_transient_sys.rhs->first_local_index(),
                            std::numeric_limits<Real>::quiet_NaN());
  _transient_sys.rhs->close();

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
#if LIBMESH_HAVE_PETSC
  // Make sure that libMesh isn't going to override our preconditioner
  _transient_sys.nonlinear_solver->jacobian = nullptr;

  PetscNonlinearSolver<Number> * petsc_nonlinear_solver =
      static_cast<PetscNonlinearSolver<Number> *>(_transient_sys.nonlinear_solver.get());

  PetscMatrix<Number> * petsc_mat = static_cast<PetscMatrix<Number> *>(_transient_sys.matrix);

  SNESSetJacobian(petsc_nonlinear_solver->snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
#if PETSC_VERSION_LESS_THAN(3, 4, 0)
                  SNESDefaultComputeJacobian,
#else
                  SNESComputeJacobianDefault,
#endif
                  nullptr);
#endif
}

void
NonlinearSystem::setupColoringFiniteDifferencedPreconditioner()
{
#ifdef LIBMESH_HAVE_PETSC
  // Make sure that libMesh isn't going to override our preconditioner
  _transient_sys.nonlinear_solver->jacobian = nullptr;

  PetscNonlinearSolver<Number> & petsc_nonlinear_solver =
      dynamic_cast<PetscNonlinearSolver<Number> &>(*_transient_sys.nonlinear_solver);

  // Pointer to underlying PetscMatrix type
  PetscMatrix<Number> * petsc_mat = dynamic_cast<PetscMatrix<Number> *>(_transient_sys.matrix);

#if PETSC_VERSION_LESS_THAN(3, 2, 0)
  // This variable is only needed for PETSC < 3.2.0
  PetscVector<Number> * petsc_vec =
      dynamic_cast<PetscVector<Number> *>(_transient_sys.solution.get());
#endif

  Moose::compute_jacobian(*_transient_sys.current_local_solution, *petsc_mat, _transient_sys);

  if (!petsc_mat)
    mooseError("Could not convert to Petsc matrix.");

  petsc_mat->close();

  PetscErrorCode ierr = 0;
  ISColoring iscoloring;

#if PETSC_VERSION_LESS_THAN(3, 2, 0)
  // PETSc 3.2.x
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORING_LF, &iscoloring);
  CHKERRABORT(libMesh::COMM_WORLD, ierr);
#elif PETSC_VERSION_LESS_THAN(3, 5, 0)
  // PETSc 3.3.x, 3.4.x
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORINGLF, &iscoloring);
  CHKERRABORT(_communicator.get(), ierr);
#else
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
#endif

  MatFDColoringCreate(petsc_mat->mat(), iscoloring, &_fdcoloring);
  MatFDColoringSetFromOptions(_fdcoloring);
  // clang-format off
  MatFDColoringSetFunction(_fdcoloring,
                           (PetscErrorCode(*)(void))(void (*)(void)) &
                               libMesh::libmesh_petsc_snes_fd_residual,
                           &petsc_nonlinear_solver);
  // clang-format on
#if !PETSC_RELEASE_LESS_THAN(3, 5, 0)
  MatFDColoringSetUp(petsc_mat->mat(), iscoloring, _fdcoloring);
#endif
#if PETSC_VERSION_LESS_THAN(3, 4, 0)
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESDefaultComputeJacobianColor,
                  _fdcoloring);
#else
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESComputeJacobianDefaultColor,
                  _fdcoloring);
#endif
#if PETSC_VERSION_LESS_THAN(3, 2, 0)
  Mat my_mat = petsc_mat->mat();
  MatStructure my_struct;

  SNESComputeJacobian(
      petsc_nonlinear_solver.snes(), petsc_vec->vec(), &my_mat, &my_mat, &my_struct);
#endif

#if PETSC_VERSION_LESS_THAN(3, 2, 0)
  ISColoringDestroy(iscoloring);
#else
  // PETSc 3.3.0
  ISColoringDestroy(&iscoloring);
#endif

#endif
}

bool
NonlinearSystem::converged()
{
  if (_fe_problem.hasException())
    return false;

  return _transient_sys.nonlinear_solver->converged;
}
