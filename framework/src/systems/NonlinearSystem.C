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
#include "TimedPrint.h"
#include "MooseVariableScalar.h"
#include "MooseTypes.h"

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
        fe_problem, fe_problem.es().add_system<TransientNonlinearImplicitSystem>(name), name),
    _transient_sys(fe_problem.es().get_system<TransientNonlinearImplicitSystem>(name)),
    _nl_residual_functor(_fe_problem),
    _fd_residual_functor(_fe_problem),
    _use_coloring_finite_difference(false),
    _auto_scaling_initd(false)
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

  /// Forcefully init the default solution states to match those available in libMesh
  /// Must be called here because it would call virtuals in the parent class
  solutionState(_default_solution_states);
}

NonlinearSystem::~NonlinearSystem() {}

void
NonlinearSystem::init()
{
  NonlinearSystemBase::init();

  if (_automatic_scaling && _resid_vs_jac_scaling_param < 1. - TOLERANCE)
    // Add diagonal matrix that will be used for computing scaling factors
    _transient_sys.add_matrix<DiagonalMatrix>("scaling_matrix");
}

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
    CONSOLE_TIMED_PRINT("Computing initial residual")
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

void
NonlinearSystem::setupScalingGrouping()
{
  if (_auto_scaling_initd)
    return;

  const auto & field_variables = _vars[0].fieldVariables();

  if (_scaling_group_variables.empty())
  {
    _var_to_group_var.reserve(field_variables.size());
    _num_scaling_groups = field_variables.size();

    for (auto & field_var : field_variables)
      _var_to_group_var.insert(std::make_pair(field_var->number(), field_var->number()));
  }
  else
  {
    std::set<unsigned int> var_numbers, var_numbers_covered, var_numbers_not_covered;
    for (const auto & field_var : field_variables)
      var_numbers.insert(field_var->number());

    _num_scaling_groups = _scaling_group_variables.size();

    for (MooseIndex(_scaling_group_variables) group_index = 0;
         group_index < _scaling_group_variables.size();
         ++group_index)
      for (const auto & var_name : _scaling_group_variables[group_index])
      {
        auto & fe_var = getVariable(/*tid=*/0, var_name);
        auto map_pair = _var_to_group_var.insert(std::make_pair(fe_var.number(), group_index));
        if (!map_pair.second)
          mooseError("Variable ", var_name, " is contained in multiple scaling grouplings");
        var_numbers_covered.insert(fe_var.number());
      }

    std::set_difference(var_numbers.begin(),
                        var_numbers.end(),
                        var_numbers_covered.begin(),
                        var_numbers_covered.end(),
                        std::inserter(var_numbers_not_covered, var_numbers_not_covered.begin()));

    _num_scaling_groups = _scaling_group_variables.size() + var_numbers_not_covered.size();

    auto index = static_cast<unsigned int>(_scaling_group_variables.size());
    for (auto var_number : var_numbers_not_covered)
      _var_to_group_var.insert(std::make_pair(var_number, index++));
  }
}

void
NonlinearSystem::computeScaling()
{
  _console << "\nPerforming automatic scaling calculation\n\n";

  TIME_SECTION(_compute_scaling_timer);

  // container for repeated access of element global dof indices
  std::vector<dof_id_type> dof_indices;

  if (!_auto_scaling_initd)
    setupScalingGrouping();

  auto & field_variables = _vars[0].fieldVariables();
  auto & scalar_variables = _vars[0].scalars();

  std::vector<Real> inverse_scaling_factors(_num_scaling_groups + scalar_variables.size(), 0);
  std::vector<Real> resid_inverse_scaling_factors(_num_scaling_groups + scalar_variables.size(), 0);
  std::vector<Real> jac_inverse_scaling_factors(_num_scaling_groups + scalar_variables.size(), 0);
  auto & dof_map = dofMap();

  // what types of scaling do we want?
  bool jac_scaling = _resid_vs_jac_scaling_param < 1. - TOLERANCE;
  bool resid_scaling = _resid_vs_jac_scaling_param > TOLERANCE;

  SparseMatrix<Number> * scaling_matrix = nullptr;
  NumericVector<Number> * scaling_residual = nullptr;

  if (jac_scaling)
  {
    mooseAssert(_transient_sys.have_matrix("scaling_matrix"),
                "The scaling matrix has not been created. There must have been an issue in "
                "initialization of the NonlinearSystem");

    scaling_matrix = &_transient_sys.get_matrix("scaling_matrix");

    if (!_auto_scaling_initd)
    {
      auto init_vector = NumericVector<Number>::build(this->comm());
      init_vector->init(system().n_dofs(), system().n_local_dofs(), /*fast=*/false, PARALLEL);

      auto diagonal_matrix = static_cast<DiagonalMatrix<Number> *>(scaling_matrix);
      diagonal_matrix->init(*init_vector);
    }

    _computing_scaling_jacobian = true;
    _fe_problem.computeJacobianSys(_transient_sys, *_current_solution, *scaling_matrix);
    _computing_scaling_jacobian = false;
  }

  if (resid_scaling)
  {
    scaling_residual = _transient_sys.rhs;

    _computing_scaling_residual = true;
    _fe_problem.computingNonlinearResid(true);
    _fe_problem.computeResidualSys(_transient_sys, *_current_solution, *scaling_residual);
    _fe_problem.computingNonlinearResid(false);
    _computing_scaling_residual = false;
  }

  // Compute our scaling factors for the spatial field variables
  for (const auto & elem : *mesh().getActiveLocalElementRange())
    for (MooseIndex(field_variables) i = 0; i < field_variables.size(); ++i)
    {
      auto & field_variable = *field_variables[i];
      auto var_number = field_variable.number();
      dof_map.dof_indices(elem, dof_indices, var_number);
      for (auto dof_index : dof_indices)
        if (dof_map.local_index(dof_index))
        {
          if (jac_scaling)
          {
            // For now we will use the diagonal for determining scaling
            auto mat_value = (*scaling_matrix)(dof_index, dof_index);
            auto & factor = jac_inverse_scaling_factors[_var_to_group_var[var_number]];
            factor = std::max(factor, std::abs(mat_value));
          }
          if (resid_scaling)
          {
            auto vec_value = (*scaling_residual)(dof_index);
            auto & factor = resid_inverse_scaling_factors[_var_to_group_var[var_number]];
            factor = std::max(factor, std::abs(vec_value));
          }
        }
    }

  auto offset = _num_scaling_groups;

  // Compute scalar factors for scalar variables
  for (MooseIndex(scalar_variables) i = 0; i < scalar_variables.size(); ++i)
  {
    auto & scalar_variable = *scalar_variables[i];
    dof_map.SCALAR_dof_indices(dof_indices, scalar_variable.number());
    for (auto dof_index : dof_indices)
      if (dof_map.local_index(dof_index))
      {
        if (jac_scaling)
        {
          // For now we will use the diagonal for determining scaling
          auto mat_value = (*scaling_matrix)(dof_index, dof_index);
          jac_inverse_scaling_factors[offset + i] =
              std::max(jac_inverse_scaling_factors[offset + i], std::abs(mat_value));
        }
        if (resid_scaling)
        {
          auto vec_value = (*scaling_residual)(dof_index);
          resid_inverse_scaling_factors[offset + i] =
              std::max(resid_inverse_scaling_factors[offset + i], std::abs(vec_value));
        }
      }
  }

  if (resid_scaling)
    _communicator.max(resid_inverse_scaling_factors);
  if (jac_scaling)
    _communicator.max(jac_inverse_scaling_factors);

  if (jac_scaling && resid_scaling)
    for (MooseIndex(inverse_scaling_factors) i = 0; i < inverse_scaling_factors.size(); ++i)
    {
      // Be careful not to take log(0)
      if (!resid_inverse_scaling_factors[i])
      {
        if (!jac_inverse_scaling_factors[i])
          inverse_scaling_factors[i] = 1;
        else
          inverse_scaling_factors[i] = jac_inverse_scaling_factors[i];
      }
      else if (!jac_inverse_scaling_factors[i])
        // We know the resid is not zero
        inverse_scaling_factors[i] = resid_inverse_scaling_factors[i];
      else
        inverse_scaling_factors[i] =
            std::exp(_resid_vs_jac_scaling_param * std::log(resid_inverse_scaling_factors[i]) +
                     (1 - _resid_vs_jac_scaling_param) * std::log(jac_inverse_scaling_factors[i]));
    }
  else if (jac_scaling)
    inverse_scaling_factors = jac_inverse_scaling_factors;
  else if (resid_scaling)
    inverse_scaling_factors = resid_inverse_scaling_factors;
  else
    mooseError("We shouldn't be calling this routine if we're not performing any scaling");

  // We have to make sure that our scaling values are not zero
  for (auto & scaling_factor : inverse_scaling_factors)
    if (scaling_factor < std::numeric_limits<Real>::epsilon())
      scaling_factor = 1;

  // Now flatten the group scaling factors to the individual variable scaling factors
  std::vector<Real> flattened_inverse_scaling_factors(field_variables.size() +
                                                      scalar_variables.size());
  for (MooseIndex(field_variables) i = 0; i < field_variables.size(); ++i)
    flattened_inverse_scaling_factors[i] = inverse_scaling_factors[_var_to_group_var[i]];
  for (MooseIndex(scalar_variables) i = 0; i < scalar_variables.size(); ++i)
    flattened_inverse_scaling_factors[i + offset] = inverse_scaling_factors[i + offset];

  // Now set the scaling factors for the variables
  applyScalingFactors(flattened_inverse_scaling_factors);
  if (auto displaced_problem = _fe_problem.getDisplacedProblem().get())
    displaced_problem->systemBaseNonlinear().applyScalingFactors(flattened_inverse_scaling_factors);

  _auto_scaling_initd = true;
}

void
NonlinearSystem::attachPreconditioner(Preconditioner<Number> * preconditioner)
{
  nonlinearSolver()->attach_preconditioner(preconditioner);
}
