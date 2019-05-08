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
    _explicit_residual(_nl.addVector("explicit_residual", false, PARALLEL)),
    _explicit_euler_update(_nl.addVector("explicit_euler_update", true, PARALLEL)),
    _mass_matrix_diag(_nl.addVector("mass_matrix_diag", false, PARALLEL))
{
  _Ke_time_tag = _fe_problem.getMatrixTagID("TIME");

  // Try to keep MOOSE from doing any nonlinear stuff
  _fe_problem.solverParams()._type = Moose::ST_LINEAR;

  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    _ones = &_nl.addVector("ones", false, PARALLEL);
}

void
ActuallyExplicitEuler::initialSetup()
{
  meshChanged();
}

void
ActuallyExplicitEuler::init()
{
}

void
ActuallyExplicitEuler::preSolve()
{
}

void
ActuallyExplicitEuler::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("ActuallyExplicitEuler: Time derivative of solution (`u_dot`) is not stored. Please "
               "set uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();

  _du_dot_du = 1.0 / _dt;
}

void
ActuallyExplicitEuler::computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
ActuallyExplicitEuler::solve()
{
  auto & es = _fe_problem.es();

  auto & nonlinear_system = _fe_problem.getNonlinearSystemBase();

  auto & libmesh_system = dynamic_cast<NonlinearImplicitSystem &>(nonlinear_system.system());

  auto & mass_matrix = *libmesh_system.matrix;

  _current_time = _fe_problem.time();

  // Set time back so that we're evaluating the interior residual at the old time
  _fe_problem.time() = _fe_problem.timeOld();

  libmesh_system.update();

  // Must compute the residual first
  _explicit_residual.zero();
  _fe_problem.computeResidual(*libmesh_system.current_local_solution, _explicit_residual);

  // The residual is on the RHS
  _explicit_residual *= -1.;

  // Compute the mass matrix
  _fe_problem.computeJacobianTag(*libmesh_system.current_local_solution, mass_matrix, _Ke_time_tag);

  // Still testing whether leaving the old update is a good idea or not
  // _explicit_euler_update = 0;

  auto converged = false;

  switch (_solve_type)
  {
    case CONSISTENT:
    {
      const auto num_its_and_final_tol = _linear_solver->solve(
          mass_matrix,
          _explicit_euler_update,
          _explicit_residual,
          es.parameters.get<Real>("linear solver tolerance"),
          es.parameters.get<unsigned int>("linear solver maximum iterations"));

      converged = checkLinearConvergence();

      _n_linear_iterations = num_its_and_final_tol.first;

      break;
    }
    case LUMPED:
    {
      // Computes the sum of each row (lumping)
      // Note: This is actually how PETSc does it
      // It's not "perfectly optimal" - but it will be fast (and universal)
      mass_matrix.vector_mult(_mass_matrix_diag, *_ones);

      // "Invert" the diagonal mass matrix
      _mass_matrix_diag.reciprocal();

      // Multiply the inversion by the RHS
      _explicit_euler_update.pointwise_mult(_mass_matrix_diag, _explicit_residual);

      // Check for convergence by seeing if there is a nan or inf
      auto sum = _explicit_euler_update.sum();
      converged = std::isfinite(sum);

      _n_linear_iterations = 0;

      break;
    }
    case LUMP_PRECONDITIONED:
    {
      mass_matrix.vector_mult(_mass_matrix_diag, *_ones);
      _mass_matrix_diag.reciprocal();

      const auto num_its_and_final_tol = _linear_solver->solve(
          mass_matrix,
          _explicit_euler_update,
          _explicit_residual,
          es.parameters.get<Real>("linear solver tolerance"),
          es.parameters.get<unsigned int>("linear solver maximum iterations"));

      converged = checkLinearConvergence();

      _n_linear_iterations = num_its_and_final_tol.first;

      break;
    }
    default:
      mooseError("Unknown solve_type in ActuallyExplicitEuler ");
  }

  *libmesh_system.solution = nonlinear_system.solutionOld();
  *libmesh_system.solution += _explicit_euler_update;

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
  _fe_problem.time() = _current_time;
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
    _preconditioner = libmesh_make_unique<LumpedPreconditioner>(_mass_matrix_diag);
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
