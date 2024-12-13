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

// libMesh includes
#include "libmesh/nonlinear_solver.h"

using namespace libMesh;

registerMooseObject("MooseApp", ActuallyExplicitEuler);

InputParameters
ActuallyExplicitEuler::validParams()
{
  InputParameters params = ExplicitTimeIntegrator::validParams();

  params.addClassDescription(
      "Implementation of Explicit/Forward Euler without invoking any of the nonlinear solver");

  params.addParam<bool>("use_constant_mass",
                        false,
                        "If set to true, will only compute the mass matrix in the first time step, "
                        "and keep using it throughout the simulation.");

  return params;
}

ActuallyExplicitEuler::ActuallyExplicitEuler(const InputParameters & parameters)
  : ExplicitTimeIntegrator(parameters), _constant_mass(getParam<bool>("use_constant_mass"))
{
}

void
ActuallyExplicitEuler::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("ActuallyExplicitEuler: Time derivative of solution (`u_dot`) is not stored. Please "
               "set uDotRequested() to true in FEProblemBase before requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();
  computeDuDotDu();
}

void
ActuallyExplicitEuler::computeADTimeDerivatives(ADReal & ad_u_dot,
                                                const dof_id_type & dof,
                                                ADReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
ActuallyExplicitEuler::solve()
{
  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  _current_time = _fe_problem.time();

  // Set time to the time at which to evaluate the residual
  _fe_problem.time() = _fe_problem.timeOld();
  _nonlinear_implicit_system->update();

  // Compute the residual
  _explicit_residual->zero();
  _fe_problem.computeResidual(
      *_nonlinear_implicit_system->current_local_solution, *_explicit_residual, _nl->number());

  // Move the residual to the RHS
  *_explicit_residual *= -1.0;

  // Compute the mass matrix
  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();
  if (!_constant_mass || (_constant_mass && _t_step == 1))
    _fe_problem.computeJacobianTag(
        *_nonlinear_implicit_system->current_local_solution, mass_matrix, _Ke_time_tag);

  // Perform the linear solve
  bool converged = performExplicitSolve(mass_matrix);

  // Update the solution
  *_nonlinear_implicit_system->solution = _nl->solutionOld();
  *_nonlinear_implicit_system->solution += *_solution_update;

  // Constraints may be solved in an uncoupled way. For example, momentum-balance equations may be
  // solved node-wise and then the solution (e.g. velocities or positions)can be applied to those
  // nodes without solving for such constraints on a system level. This strategy is being used for
  // node-face contact in explicit dynamics.
  _nl->overwriteNodeFace(*_nonlinear_implicit_system->solution);

  // Enforce contraints on the solution
  DofMap & dof_map = _nonlinear_implicit_system->get_dof_map();
  dof_map.enforce_constraints_exactly(*_nonlinear_implicit_system,
                                      _nonlinear_implicit_system->solution.get());
  _nonlinear_implicit_system->update();

  _nl->setSolution(*_nonlinear_implicit_system->current_local_solution);

  _nonlinear_implicit_system->nonlinear_solver->converged = converged;
}

void
ActuallyExplicitEuler::postResidual(NumericVector<Number> & residual)
{
  residual += *_Re_time;
  residual += *_Re_non_time;
  residual.close();

  // Reset time to the time at which to evaluate nodal BCs, which comes next
  _fe_problem.time() = _current_time;
}
