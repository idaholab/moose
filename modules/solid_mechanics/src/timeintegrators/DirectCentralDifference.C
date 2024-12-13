//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Assembly.h"
#include "DirectCentralDifference.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
// libMesh includes
#include "libmesh/nonlinear_solver.h"
#include "libmesh/sparse_matrix.h"
#include "DirichletBCBase.h"
#include "libmesh/vector_value.h"

registerMooseObject("SolidMechanicsApp", DirectCentralDifference);

InputParameters
DirectCentralDifference::validParams()
{
  InputParameters params = ExplicitTimeIntegrator::validParams();

  params.addClassDescription(
      "Implementation of Explicit/Forward Euler without invoking any of the nonlinear solver");

  params.addParam<bool>("use_constant_mass",
                        false,
                        "If set to true, will only compute the mass matrix in the first time step, "
                        "and keep using it throughout the simulation.");
  params.addParam<TagName>("mass_matrix_tag", "mass", "The tag for the mass matrix");

  MooseEnum solve_type("consistent lumped lump_preconditioned", "lumped");
  params.setParameters("solve_type", solve_type);
  params.ignoreParameter<MooseEnum>("solve_type");
  return params;
}

DirectCentralDifference::DirectCentralDifference(const InputParameters & parameters)
  : ExplicitTimeIntegrator(parameters),
    _constant_mass(getParam<bool>("use_constant_mass")),
    _mass_matrix(getParam<TagName>("mass_matrix_tag"))
{
  _fe_problem.setUDotRequested(true);
  _fe_problem.setUDotOldRequested(true);
  _fe_problem.setUDotDotRequested(true);
  _fe_problem.setUDotDotOldRequested(true);
}

void
DirectCentralDifference::computeTimeDerivatives()
{
  /*
  Because this is called in NonLinearSystemBase
  this should not actually compute the time derivatives.
  Calculating time derivatives here will cause issues for the
  solution update.
  */
  return;
}

void
DirectCentralDifference::solve()
{
  // Getting the tagID for the mass matrix
  auto mass_tag = _sys.subproblem().getMatrixTagID(_mass_matrix);

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  _current_time = _fe_problem.time();

  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();

  // Compute the mass matrix
  if (!_constant_mass || (_constant_mass && _t_step == 1))
    _fe_problem.computeJacobianTag(
        *_nonlinear_implicit_system->current_local_solution, mass_matrix, mass_tag);

  // Set time to the time at which to evaluate the residual
  _fe_problem.time() = _fe_problem.timeOld();
  _nonlinear_implicit_system->update();

  // Calculating the lumped mass matrix for use in residual calculation
  mass_matrix.vector_mult(*_mass_matrix_diag, *_ones);

  // Compute the residual
  _explicit_residual->zero();
  _fe_problem.computeResidual(
      *_nonlinear_implicit_system->current_local_solution, *_explicit_residual, _nl->number());

  // Move the residual to the RHS
  *_explicit_residual *= -1.0;

  // Perform the linear solve
  bool converged = performExplicitSolve(mass_matrix);
  _nl->overwriteNodeFace(*_nonlinear_implicit_system->solution);

  // Update the solution
  *_nonlinear_implicit_system->solution = _nl->solutionOld();
  *_nonlinear_implicit_system->solution += *_solution_update;

  _nonlinear_implicit_system->update();

  _nl->setSolution(*_nonlinear_implicit_system->current_local_solution);
  _nonlinear_implicit_system->nonlinear_solver->converged = converged;
}

void
DirectCentralDifference::postResidual(NumericVector<Number> & residual)
{
  residual += *_Re_time;
  residual += *_Re_non_time;
  residual.close();

  // Reset time to the time at which to evaluate nodal BCs, which comes next
  _fe_problem.time() = _current_time;
}

bool
DirectCentralDifference::performExplicitSolve(SparseMatrix<Number> &)
{
  bool converged = false;

  // "Invert" the diagonal mass matrix
  _mass_matrix_diag->reciprocal();
  // Calculate acceleration
  auto & accel = *_sys.solutionUDotDot();
  accel.pointwise_mult(*_mass_matrix_diag, *_explicit_residual);

  // Scaling the acceleration
  auto accel_scaled = accel.clone();
  accel_scaled->scale((_dt + _dt_old) / 2);

  // Adding old vel to new vel
  auto & vel = *_sys.solutionUDot();
  const auto & old_vel = _sys.solutionUDotOld();

  vel = *old_vel->clone();
  vel += *accel_scaled;

  *_solution_update = vel;
  _solution_update->scale(_dt);

  // Check for convergence by seeing if there is a nan or inf
  auto sum = _solution_update->sum();
  converged = std::isfinite(sum);

  // The linear iteration count remains zero
  _n_linear_iterations = 0;
  vel.close();
  accel.close();
  return converged;
}
