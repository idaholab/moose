//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExplicitSSPRungeKutta.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/nonlinear_solver.h"

using namespace libMesh;

registerMooseObject("MooseApp", ExplicitSSPRungeKutta);

InputParameters
ExplicitSSPRungeKutta::validParams()
{
  InputParameters params = ExplicitTimeIntegrator::validParams();

  MooseEnum orders("1=1 2 3");
  params.addRequiredParam<MooseEnum>("order", orders, "Order of time integration");
  params.addClassDescription("Explicit strong stability preserving Runge-Kutta methods");
  return params;
}

ExplicitSSPRungeKutta::ExplicitSSPRungeKutta(const InputParameters & parameters)
  : ExplicitTimeIntegrator(parameters),
    _order(getParam<MooseEnum>("order")),
    _stage(0),
    _solution_intermediate_stage(addVector("solution_intermediate_stage", false, GHOSTED)),
    _tmp_solution(addVector("tmp_solution", false, GHOSTED)),
    _tmp_mass_solution_product(addVector("tmp_mass_solution_product", false, GHOSTED))
{
  // For SSPRK methods up to order 3, the number of stages equals the order
  _n_stages = _order;

  if (_order == 1)
  {
    _a = {{1.0}};
    _b = {1.0};
    _c = {0.0};
  }
  else if (_order == 2)
  {
    _a = {{1.0, 0.0}, {0.5, 0.5}};
    _b = {1.0, 0.5};
    _c = {0.0, 1.0};
  }
  else if (_order == 3)
  {
    _a = {{1.0, 0.0, 0.0}, {0.75, 0.25, 0.0}, {1.0 / 3.0, 0.0, 2.0 / 3.0}};
    _b = {1.0, 0.25, 2.0 / 3.0};
    _c = {0.0, 1.0, 0.5};
  }
  else
    mooseError("Invalid time integration order.");

  _solution_stage.resize(_n_stages + 1, nullptr);
}

void
ExplicitSSPRungeKutta::computeTimeDerivatives()
{
  // Only the Jacobian needs to be computed, since the mass matrix needs it
  computeDuDotDu();
}

void
ExplicitSSPRungeKutta::computeADTimeDerivatives(ADReal & ad_u_dot,
                                                const dof_id_type & dof,
                                                ADReal & /*ad_u_dotdot*/) const
{
  // Note that if the solution for the current stage is not a nullptr, then neither
  // are the previous stages.
  if (_solution_stage[_stage])
  {
    for (unsigned int k = 0; k <= _stage; k++)
      ad_u_dot -= _a[_stage][k] * (*(_solution_stage[k]))(dof);
    ad_u_dot *= 1.0 / (_b[_stage] * _dt);
  }
  else
  {
    // We must be outside the solve loop in order to meet this criterion. In that case are we at
    // timestep_begin or timestep_end? We don't know, so I don't think it's meaningful to compute
    // derivatives here. Let's put in a quiet NaN which will only signal if we try to do something
    // meaningful with it (and then we do want to signal because time derivatives may not be
    // meaningful right now)
    ad_u_dot = std::numeric_limits<typename ADReal::value_type>::quiet_NaN();
  }
}

void
ExplicitSSPRungeKutta::solve()
{
  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  _current_time = _fe_problem.time();
  const Real time_old = _fe_problem.timeOld();
  const Real dt = _current_time - time_old;

  bool converged = false;

  _solution_stage[0] = &_solution_old;
  for (_stage = 0; _stage < _n_stages; _stage++)
  {
    if (_stage == 0)
    {
      // Nothing needs to be done
    }
    else if (_stage == _n_stages - 1)
    {
      _solution_stage[_stage] = _solution;
    }
    else
    {
      // Else must be the intermediate stage of the 3-stage method
      *_solution_intermediate_stage = *_solution;
      _solution_intermediate_stage->close();
      _solution_stage[_stage] = _solution_intermediate_stage;
    }

    // Set stage time for residual evaluation
    _fe_problem.time() = time_old + _c[_stage] * dt;
    _nonlinear_implicit_system->update();

    converged = solveStage();
    if (!converged)
      return;
  }

  if (_stage == _n_stages)
    // We made it to the end of the solve. We may call functions like computeTimeDerivatives later
    // for postprocessing purposes in which case we need to ensure we're accessing our data
    // correctly (e.g. not out-of-bounds)
    --_stage;
}

bool
ExplicitSSPRungeKutta::solveStage()
{
  // Compute the mass matrix
  computeTimeDerivatives();
  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();
  _fe_problem.computeJacobianTag(
      *_nonlinear_implicit_system->current_local_solution, mass_matrix, _Ke_time_tag);

  // Compute RHS vector using previous stage solution in steady-state residual
  _explicit_residual->zero();
  _fe_problem.computeResidual(
      *_nonlinear_implicit_system->current_local_solution, *_explicit_residual, _nl->number());

  // Move the residual to the RHS
  *_explicit_residual *= -1.0;

  // Perform the linear solve
  bool converged = performExplicitSolve(mass_matrix);

  // Update the solution: u^(s) = u^(s-1) + du^(s)
  (*_nonlinear_implicit_system->solution).zero();
  *_nonlinear_implicit_system->solution = *(_solution_stage[_stage]);
  *_nonlinear_implicit_system->solution += *_solution_update;

  // Enforce contraints on the solution
  DofMap & dof_map = _nonlinear_implicit_system->get_dof_map();
  dof_map.enforce_constraints_exactly(*_nonlinear_implicit_system,
                                      _nonlinear_implicit_system->solution.get());
  _nonlinear_implicit_system->update();

  _nl->setSolution(*_nonlinear_implicit_system->current_local_solution);

  _nonlinear_implicit_system->nonlinear_solver->converged = converged;

  return converged;
}

void
ExplicitSSPRungeKutta::postResidual(NumericVector<Number> & residual)
{
  // The time residual is not included in the steady-state residual
  residual += *_Re_non_time;

  // Compute \sum_{k=0}^{s-1} a_{s,k} u^(k) - u^(s-1)
  _tmp_solution->zero();
  for (unsigned int k = 0; k <= _stage; k++)
    _tmp_solution->add(_a[_stage][k], *(_solution_stage[k]));
  _tmp_solution->add(-1.0, *(_solution_stage[_stage]));
  _tmp_solution->close();

  // Perform mass matrix product with the above vector
  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();
  mass_matrix.vector_mult(*_tmp_mass_solution_product, *_tmp_solution);

  // Finish computing residual vector (before modification by nodal BCs)
  residual -= *_tmp_mass_solution_product;

  residual.close();

  // Set time at which to evaluate nodal BCs
  _fe_problem.time() = _current_time;
}

Real
ExplicitSSPRungeKutta::duDotDuCoeff() const
{
  return Real(1) / _b[_stage];
}
