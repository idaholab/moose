//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ActuallyExplicitEuler.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

// libMesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/preconditioner.h"
#include "libmesh/enum_convergence_flags.h"

registerMooseObject("MooseApp", ActuallyExplicitEuler);

template <>
InputParameters
validParams<ActuallyExplicitEuler>()
{
  InputParameters params = validParams<TimeIntegrator>();

  MooseEnum solve_type("consistent lumped lump_preconditioned", "consistent");

  params.addParam<MooseEnum>(
      "solve_type",
      solve_type,
      "The way to solve the system.  A 'consistent' solve uses the full mass matrix and actually "
      "needs to use a linear solver to solve the problem.  'lumped' uses a lumped mass matrix with "
      "a simple inversion - incredibly fast but may be less accurate.  'lump_preconditioned' uses "
      "the lumped mass matrix as a preconditioner for the 'consistent' solve");

  params.addClassDescription(
      "Implementation of Explicit/Forward Euler without invoking any of the nonlinear solver");

  return params;
}

/**
 * Helper class to apply preconditioner
 */
class LumpedPreconditioner : public Preconditioner<Real>
{
public:
  LumpedPreconditioner(const NumericVector<Real> & diag_inverse)
    : Preconditioner(diag_inverse.comm()), _diag_inverse(diag_inverse)
  {
  }

  virtual void init() override
  {
    // No more initialization needed here
    _is_initialized = true;
  }

  virtual void apply(const NumericVector<Real> & x, NumericVector<Real> & y) override
  {
    y.pointwise_mult(_diag_inverse, x);
  }

protected:
  /// The inverse of the diagonal of the lumped matrix
  const NumericVector<Real> & _diag_inverse;
};

ActuallyExplicitEuler::ActuallyExplicitEuler(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    MeshChangedInterface(parameters),
    _solve_type(getParam<MooseEnum>("solve_type")),
    _rhs(_nl.addVector("rhs", false, PARALLEL)),
    _explicit_residual(_nl.addVector("explicit_residual", false, PARALLEL)),
    _explicit_euler_update(_nl.addVector("explicit_euler_update", true, PARALLEL)),
    _explicit_euler_correction(_nl.addVector("explicit_euler_correction", true, PARALLEL)),
    _system_matrix_diag(_nl.addVector("system_matrix_diag", false, PARALLEL)),
    _mass_matrix_diag(_nl.addVector("mass_matrix_diag", false, PARALLEL)),
    _solution_minus_one(_nl.addVector("solution_minus_one", false, PARALLEL)),
    _du_dotdot_du(_sys.duDotDotDu())
{
  _fe_nontime_tag = _fe_problem.getVectorTagID("NONTIME");
  _Ke_time_tag = _fe_problem.getMatrixTagID("TIME");
  _Ke_second_time_tag = _fe_problem.getMatrixTagID("SECONDTIME");

  // Try to keep MOOSE from doing any nonlinear stuff
  _fe_problem.solverParams()._type = Moose::ST_LINEAR;

  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    _ones = &_nl.addVector("ones", false, PARALLEL);

  _fe_problem.setUDotRequested(true);
  _fe_problem.setUDotDotRequested(true);
  _fe_problem.setUDotOldRequested(true);
  _fe_problem.setUDotDotOldRequested(true);
}

void
ActuallyExplicitEuler::initialSetup()
{
  meshChanged();
}

void
ActuallyExplicitEuler::init()
{
  auto & es = _fe_problem.es();
  auto & nonlinear_system = _fe_problem.getNonlinearSystemBase();
  auto & libmesh_system = dynamic_cast<NonlinearImplicitSystem &>(nonlinear_system.system());
  auto & system_matrix = *libmesh_system.matrix;

  switch (_solve_type)
  {
    case CONSISTENT:
    {
      // Compute K u - F
      _explicit_residual.zero();
      _fe_problem.computeResidualType(
          *libmesh_system.current_local_solution, _explicit_residual, _fe_nontime_tag);

      // get system matrix M + C
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_time_tag);

      // Compute explicit euler correction
      // [M] {EEC} = dt * [M + C] {u_dot}
      system_matrix.vector_mult(_rhs, *nonlinear_system.solutionUDot());
      _rhs *= _dt;
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_second_time_tag);
      _explicit_euler_correction.zero();
      _linear_solver->solve(system_matrix,
                            _explicit_euler_correction,
                            _rhs,
                            es.parameters.get<Real>("linear solver tolerance"),
                            es.parameters.get<unsigned int>("linear solver maximum iterations"));

      // Compute explicit euler update
      // [M] {EEU} = {R}
      _linear_solver->solve(system_matrix,
                            _explicit_euler_update,
                            _explicit_residual,
                            es.parameters.get<Real>("linear solver tolerance"),
                            es.parameters.get<unsigned int>("linear solver maximum iterations"));
      _linear_solver->clear();

      break;
    }
    case LUMPED:
    {
      // Compute K u - F
      _explicit_residual.zero();
      _fe_problem.computeResidualType(
          *libmesh_system.current_local_solution, _explicit_residual, _fe_nontime_tag);

      // get system matrix M + C
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_time_tag);
      system_matrix.vector_mult(_system_matrix_diag, *_ones);

      // get mass matrix M and its inverse
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_second_time_tag);
      system_matrix.vector_mult(_mass_matrix_diag, *_ones);
      _mass_matrix_diag.reciprocal();

      // Compute explicit euler correction
      // [M] {EEC} = dt * [M + C] {u_dot}
      _explicit_euler_correction.zero();
      _explicit_euler_correction += *nonlinear_system.solutionUDot();
      _explicit_euler_correction *= _dt;
      _explicit_euler_correction.pointwise_mult(_explicit_euler_correction, _system_matrix_diag);
      _explicit_euler_correction.pointwise_mult(_explicit_euler_correction, _mass_matrix_diag);

      // Multiply the inversion of mass matrix by the RHS to get explicit euler update
      _explicit_euler_update.pointwise_mult(_mass_matrix_diag, _explicit_residual);

      break;
    }
    case LUMP_PRECONDITIONED:
    {
      // Compute K u - F
      _explicit_residual.zero();
      _fe_problem.computeResidualType(
          *libmesh_system.current_local_solution, _explicit_residual, _fe_nontime_tag);

      // get system matrix M + C
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_time_tag);

      // Compute explicit euler correction
      // rhs = dt * [M + C] {u_dot}
      system_matrix.vector_mult(_rhs, *nonlinear_system.solutionUDot());
      _rhs *= _dt;
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_second_time_tag);
      // At initial solve, precondition the linear solver with mass matrix instead of system matrix
      system_matrix.vector_mult(_system_matrix_diag, *_ones);
      _system_matrix_diag.reciprocal();
      // [M] {EEC} = rhs
      _explicit_euler_correction.zero();
      _linear_solver->solve(system_matrix,
                            _explicit_euler_correction,
                            _rhs,
                            es.parameters.get<Real>("linear solver tolerance"),
                            es.parameters.get<unsigned int>("linear solver maximum iterations"));

      // Compute explicit euler update
      // [M] {EEU} = {R}
      _linear_solver->solve(system_matrix,
                            _explicit_euler_update,
                            _explicit_residual,
                            es.parameters.get<Real>("linear solver tolerance"),
                            es.parameters.get<unsigned int>("linear solver maximum iterations"));
      _linear_solver->clear();

      break;
    }
    default:
      mooseError("Unknown solve_type in ActuallyExplicitEuler ");
  }

  // compute intial time derivatives
  _solution_minus_one.zero();
  _solution_minus_one += *_solution;
  _solution_minus_one -= _explicit_euler_update;
  _solution_minus_one -= _explicit_euler_correction;

  // In case mass matrix is singular
  auto sum = _solution_minus_one.sum();
  if (!std::isfinite(sum))
    _solution_minus_one.zero();

  // assertion fails without this line
  _sys.solutionUDot()->close();
}

void
ActuallyExplicitEuler::preSolve()
{
}

void
ActuallyExplicitEuler::computeTimeDerivatives()
{
  if (_t_step != 0)
  {
    NumericVector<Number> & u_dot = *_sys.solutionUDot();
    u_dot = *_sys.solutionUDotOld();
    u_dot.close();
  }
  NumericVector<Number> & u_dotdot = *_sys.solutionUDotDot();
  u_dotdot = *_sys.solutionUDotDotOld();
  u_dotdot.close();

  _du_dot_du = 1.0 / _dt;
  _du_dotdot_du = 1.0 / _dt / _dt;
}

void
ActuallyExplicitEuler::computeEETimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("ActuallyExplicitEuler: Time derivative of solution (`u_dot`) is not stored. Please "
               "set uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  if (!_sys.solutionUDotDot())
    mooseError("ActuallyExplicitEuler: Second time derivative of solution (`u_dotdot`) is not "
               "stored. Please set uDotDotRequested() to true in FEProblemBase befor requesting "
               "`u_dotdot`.");

  if (_t_step != 0)
  {
    NumericVector<Number> & u_dot = *_sys.solutionUDot();
    u_dot = *_solution;
    u_dot -= _solution_old;
    u_dot *= 1.0 / _dt;
    u_dot.close();

    NumericVector<Number> & u_dotdot = *_sys.solutionUDotDot();
    if (_t_step == 1)
      u_dotdot = _solution_minus_one;
    else
      u_dotdot = _solution_older;

    u_dotdot += *_solution;
    u_dotdot -= _solution_old;
    u_dotdot -= _solution_old;

    u_dotdot *= 1.0 / _dt / _dt;
    u_dotdot.close();
  }
}

void
ActuallyExplicitEuler::solve()
{
  auto & es = _fe_problem.es();
  auto & nonlinear_system = _fe_problem.getNonlinearSystemBase();
  auto & libmesh_system = dynamic_cast<NonlinearImplicitSystem &>(nonlinear_system.system());
  auto & system_matrix = *libmesh_system.matrix;

  // Set time back so that we're evaluating the interior residual at the old time
  _current_time = _fe_problem.time();
  _fe_problem.time() = _fe_problem.timeOld();
  libmesh_system.update();

  // Must compute the residual first
  // compute {R}
  // {R} = {f} - [K] {u}
  _fe_problem.computeResidualType(
      *libmesh_system.current_local_solution, _explicit_residual, _fe_nontime_tag);
  _explicit_residual *= -1.;

  auto converged = false;

  switch (_solve_type)
  {
    case CONSISTENT:
    {
      // get mass matrix [M]
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_second_time_tag);

      // Solve explicit euler correction EEC
      // rhs = [M] {u_old - u_older}
      _rhs.zero();
      _explicit_euler_correction.zero();
      _explicit_euler_correction += _solution_old;
      if (_t_step == 1)
        _explicit_euler_correction -= _solution_minus_one;
      else
        _explicit_euler_correction -= _solution_older;
      system_matrix.vector_mult(_rhs, _explicit_euler_correction);
      // [M + C] {EEC} = rhs
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_time_tag);
      _explicit_euler_correction.zero();
      const auto num_its_and_final_tol_EEC = _linear_solver->solve(
          system_matrix,
          _explicit_euler_correction,
          _rhs,
          es.parameters.get<Real>("linear solver tolerance"),
          es.parameters.get<unsigned int>("linear solver maximum iterations"));

      converged = checkLinearConvergence();
      _n_linear_iterations = num_its_and_final_tol_EEC.first;

      // Solve for explicit euler update EEU
      // [M + C] {EEU} = {R}
      _explicit_euler_update.zero();
      const auto num_its_and_final_tol_EEU = _linear_solver->solve(
          system_matrix,
          _explicit_euler_update,
          _explicit_residual,
          es.parameters.get<Real>("linear solver tolerance"),
          es.parameters.get<unsigned int>("linear solver maximum iterations"));

      converged = converged && checkLinearConvergence();
      _n_linear_iterations += num_its_and_final_tol_EEU.first;

      break;
    }
    case LUMPED:
    {
      // Computes the sum of each row (lumping)
      // Note: This is actually how PETSc does it
      // It's not "perfectly optimal" - but it will be fast (and universal

      // Compute the system matrix [M + C] and lump it
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_time_tag);
      system_matrix.vector_mult(_system_matrix_diag, *_ones);

      // get mass matrix [M] and lump it
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_second_time_tag);
      system_matrix.vector_mult(_mass_matrix_diag, *_ones);

      // "Invert" the lumped system matrix to get [M + C]^-1
      _system_matrix_diag.reciprocal();

      // Multiply the inversion by the RHS to get explicit euler update
      // EEU = [M + C]^-1 {R}
      _explicit_euler_update.pointwise_mult(_system_matrix_diag, _explicit_residual);

      // calculate explicit euler correction
      // EEC = [M + C]^-1 * [M] * {u_old - u_older}
      _explicit_euler_correction.zero();
      _explicit_euler_correction += _solution_old;
      if (_t_step == 1)
        _explicit_euler_correction -= _solution_minus_one;
      else
        _explicit_euler_correction -= _solution_older;
      _explicit_euler_correction.pointwise_mult(_mass_matrix_diag, _explicit_euler_correction);
      _explicit_euler_correction.pointwise_mult(_system_matrix_diag, _explicit_euler_correction);

      // Check for convergence by seeing if there is a nan or inf
      auto sum = _explicit_euler_update.sum() + _explicit_euler_correction.sum();
      converged = std::isfinite(sum);

      _n_linear_iterations = 0;

      break;
    }
    case LUMP_PRECONDITIONED:
    {
      // get mass matrix [M]
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_second_time_tag);

      // Solve explicit euler correction EEC
      // rhs = [M] {u_old - u_older}
      _rhs.zero();
      _explicit_euler_correction.zero();
      _explicit_euler_correction += _solution_old;
      if (_t_step == 1)
        _explicit_euler_correction -= _solution_minus_one;
      else
        _explicit_euler_correction -= _solution_older;
      system_matrix.vector_mult(_rhs, _explicit_euler_correction);
      // Get system matrix [M + C]
      _fe_problem.computeJacobianTag(
          *libmesh_system.current_local_solution, system_matrix, _Ke_time_tag);
      // Precondition the linear solver with system matrix
      system_matrix.vector_mult(_system_matrix_diag, *_ones);
      _system_matrix_diag.reciprocal();
      // [M + C] {EEC} = rhs
      _explicit_euler_correction.zero();
      const auto num_its_and_final_tol_EEC = _linear_solver->solve(
          system_matrix,
          _explicit_euler_correction,
          _rhs,
          es.parameters.get<Real>("linear solver tolerance"),
          es.parameters.get<unsigned int>("linear solver maximum iterations"));

      converged = checkLinearConvergence();
      _n_linear_iterations = num_its_and_final_tol_EEC.first;

      // Solve for explicit euler update EEU
      // [M + C] {EEU} = {R}
      _explicit_euler_update.zero();
      const auto num_its_and_final_tol_EEU = _linear_solver->solve(
          system_matrix,
          _explicit_euler_update,
          _explicit_residual,
          es.parameters.get<Real>("linear solver tolerance"),
          es.parameters.get<unsigned int>("linear solver maximum iterations"));

      converged = converged && checkLinearConvergence();
      _n_linear_iterations += num_its_and_final_tol_EEU.first;

      break;
    }
    default:
      mooseError("Unknown solve_type in ActuallyExplicitEuler ");
  }

  // Explicitly update the solution
  // u_{n+1} = u_{n} + EEU + EEC
  *libmesh_system.solution = _solution_old;
  *libmesh_system.solution += _explicit_euler_update;
  *libmesh_system.solution += _explicit_euler_correction;

  // Enforce contraints on the solution
  DofMap & dof_map = libmesh_system.get_dof_map();
  dof_map.enforce_constraints_exactly(libmesh_system, libmesh_system.solution.get());

  libmesh_system.update();
  nonlinear_system.setSolution(*libmesh_system.current_local_solution);
  libmesh_system.nonlinear_solver->converged = converged;
}

void
ActuallyExplicitEuler::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();

  // Reset time - the boundary conditions (which is what comes next) are applied at the final time
  if (_t_step > 0)
    _fe_problem.time() = _current_time;
}

void
ActuallyExplicitEuler::postStep()
{
  computeEETimeDerivatives();
}

void
ActuallyExplicitEuler::meshChanged()
{
  // Can only be done after the system is inited
  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    *_ones = 1.;

  if (_solve_type == CONSISTENT || _solve_type == LUMP_PRECONDITIONED)
    _linear_solver = LinearSolver<Number>::build(comm());

  if (_solve_type == LUMP_PRECONDITIONED)
  {
    _preconditioner = libmesh_make_unique<LumpedPreconditioner>(_system_matrix_diag);
    _linear_solver->attach_preconditioner(_preconditioner.get());
    _linear_solver->init();
  }

  if (_solve_type == CONSISTENT || _solve_type == LUMP_PRECONDITIONED)
    Moose::PetscSupport::setLinearSolverDefaults(_fe_problem, *_linear_solver);
}

bool
ActuallyExplicitEuler::checkLinearConvergence()
{
  auto reason = _linear_solver->get_converged_reason();

  switch (reason)
  {
    case CONVERGED_RTOL_NORMAL:
    case CONVERGED_ATOL_NORMAL:
    case CONVERGED_RTOL:
    case CONVERGED_ATOL:
    case CONVERGED_ITS:
    case CONVERGED_CG_NEG_CURVE:
    case CONVERGED_CG_CONSTRAINED:
    case CONVERGED_STEP_LENGTH:
    case CONVERGED_HAPPY_BREAKDOWN:
      return true;
    case DIVERGED_NULL:
    case DIVERGED_ITS:
    case DIVERGED_DTOL:
    case DIVERGED_BREAKDOWN:
    case DIVERGED_BREAKDOWN_BICG:
    case DIVERGED_NONSYMMETRIC:
    case DIVERGED_INDEFINITE_PC:
    case DIVERGED_NAN:
    case DIVERGED_INDEFINITE_MAT:
    case CONVERGED_ITERATING:
    case DIVERGED_PCSETUP_FAILED:
      return false;
    default:
      mooseError("Unknown convergence flat in ActuallyExplicitEuler");
  }
}
