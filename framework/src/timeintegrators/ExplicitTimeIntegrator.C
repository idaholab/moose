//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExplicitTimeIntegrator.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

// libMesh includes
#include "libmesh/enum_convergence_flags.h"

InputParameters
ExplicitTimeIntegrator::validParams()
{
  InputParameters params = TimeIntegrator::validParams();

  MooseEnum solve_type("consistent lumped lump_preconditioned", "consistent");

  params.addParam<MooseEnum>(
      "solve_type",
      solve_type,
      "The way to solve the system.  A 'consistent' solve uses the full mass matrix and actually "
      "needs to use a linear solver to solve the problem.  'lumped' uses a lumped mass matrix with "
      "a simple inversion - incredibly fast but may be less accurate.  'lump_preconditioned' uses "
      "the lumped mass matrix as a preconditioner for the 'consistent' solve");

  return params;
}

ExplicitTimeIntegrator::ExplicitTimeIntegrator(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    MeshChangedInterface(parameters),

    _solve_type(getParam<MooseEnum>("solve_type")),
    _explicit_residual(_nl.addVector("explicit_residual", false, PARALLEL)),
    _solution_update(_nl.addVector("solution_update", true, PARALLEL)),
    _mass_matrix_diag(_nl.addVector("mass_matrix_diag", false, PARALLEL))
{
  _Ke_time_tag = _fe_problem.getMatrixTagID("TIME");

  // This effectively changes the default solve_type to LINEAR instead of PJFNK,
  // so that it is valid to not supply solve_type in the Executioner block:
  _fe_problem.solverParams()._type = Moose::ST_LINEAR;

  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    _ones = &_nl.addVector("ones", false, PARALLEL);
}

void
ExplicitTimeIntegrator::initialSetup()
{
  meshChanged();
}

void
ExplicitTimeIntegrator::init()
{
  if (_fe_problem.solverParams()._type != Moose::ST_LINEAR)
    mooseError(
        "The chosen time integrator requires 'solve_type = LINEAR' in the Executioner block.");
}

void
ExplicitTimeIntegrator::preSolve()
{
}

void
ExplicitTimeIntegrator::meshChanged()
{
  // Can only be done after the system is initialized
  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    *_ones = 1.;

  if (_solve_type == CONSISTENT || _solve_type == LUMP_PRECONDITIONED)
    _linear_solver = LinearSolver<Number>::build(comm());

  if (_solve_type == LUMP_PRECONDITIONED)
  {
    _preconditioner = std::make_unique<LumpedPreconditioner>(_mass_matrix_diag);
    _linear_solver->attach_preconditioner(_preconditioner.get());
    _linear_solver->init();
  }

  if (_solve_type == CONSISTENT || _solve_type == LUMP_PRECONDITIONED)
    Moose::PetscSupport::setLinearSolverDefaults(_fe_problem, *_linear_solver);
}

bool
ExplicitTimeIntegrator::performExplicitSolve(SparseMatrix<Number> & mass_matrix)
{
  bool converged = false;

  switch (_solve_type)
  {
    case CONSISTENT:
    {
      converged = solveLinearSystem(mass_matrix);

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
      _solution_update.pointwise_mult(_mass_matrix_diag, _explicit_residual);

      // Check for convergence by seeing if there is a nan or inf
      auto sum = _solution_update.sum();
      converged = std::isfinite(sum);

      // The linear iteration count remains zero
      _n_linear_iterations = 0;

      break;
    }
    case LUMP_PRECONDITIONED:
    {
      mass_matrix.vector_mult(_mass_matrix_diag, *_ones);
      _mass_matrix_diag.reciprocal();

      converged = solveLinearSystem(mass_matrix);

      break;
    }
    default:
      mooseError("Unknown solve_type in ExplicitTimeIntegrator.");
  }

  return converged;
}

bool
ExplicitTimeIntegrator::solveLinearSystem(SparseMatrix<Number> & mass_matrix)
{
  auto & es = _fe_problem.es();

  const auto num_its_and_final_tol =
      _linear_solver->solve(mass_matrix,
                            _solution_update,
                            _explicit_residual,
                            es.parameters.get<Real>("linear solver tolerance"),
                            es.parameters.get<unsigned int>("linear solver maximum iterations"));

  _n_linear_iterations += num_its_and_final_tol.first;

  const bool converged = checkLinearConvergence();

  return converged;
}

bool
ExplicitTimeIntegrator::checkLinearConvergence()
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
      mooseError("Unknown convergence reason in ExplicitTimeIntegrator.");
  }
}
