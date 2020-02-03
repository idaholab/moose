//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CentralDifference.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/nonlinear_solver.h"

registerMooseObject("MooseApp", CentralDifference);

template <>
InputParameters
validParams<CentralDifference>()
{
  InputParameters params = validParams<ActuallyExplicitEuler>();

  params.addClassDescription("Implementation of explicit, Central Difference integration without "
                             "invoking any of the nonlinear solver");

  return params;
}

CentralDifference::CentralDifference(const InputParameters & parameters)
  : ActuallyExplicitEuler(parameters),
    _du_dotdot_du(_sys.duDotDotDu()),
    _u_dotdot_residual(_sys.addVector("u_dotdot_residual", true, GHOSTED)),
    _u_dot_residual(_sys.addVector("u_dot_residual", true, GHOSTED))
{
  _is_explicit = true;
  if (_solve_type == LUMPED)
    _is_lumped = true;

  _fe_problem.setUDotOldRequested(true);
  _fe_problem.setUDotDotRequested(true);
  _fe_problem.setUDotDotOldRequested(true);
  _fe_problem.setSolutionState(4);
}

void
CentralDifference::computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof) const
{
  const auto & u_old_old_old = (*_sys.solutionState(3))(dof);

  auto u_dotdot = ad_u_dot; // TODO: Ask Alex if this has to be ad_u_dotdot. Currently, it will
                            // change u_dotdot when called.

  computeTimeDerivativeHelper(ad_u_dot, u_dotdot, _solution_old(dof), _solution_older(dof), u_old_old_old);
}

void
CentralDifference::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("CentralDifference: Time derivative of solution (`u_dot`) is not stored. Please "
               "set uDotRequested() to true in FEProblemBase before requesting `u_dot`.");

  if (!_sys.solutionUDotDot())
    mooseError("CentralDifference: Time derivative of solution (`u_dotdot`) is not stored. Please "
               "set uDotDotRequested() to true in FEProblemBase before requesting `u_dot`.");

  // Declaring u_dot and u_dotdot
  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  NumericVector<Number> & u_dotdot = *_sys.solutionUDotDot();
  NumericVector<Number> & u_old_old_old = *_sys.solutionState(3);

  // Computing derivatives
  computeTimeDerivativeHelper(u_dot, u_dotdot, _solution_old, _solution_older, u_old_old_old);

  // make sure _u_dotdot and _u_dot are in good state
  u_dotdot.close();
  u_dot.close();

  // used for Jacobian calculations
  _du_dot_du = 1.0 / (2 * _dt);
  _du_dotdot_du = 1.0 / (_dt * _dt);

  // Computing udotdot residual
  // u_dotdot_residual = u_dotdot - (u - u_old)/dt^2 = (u - 2* u_old + u_older - u + u_old) / dt^2
  // u_dotdot_residual = (u_older - u_old)/dt^2
  _u_dotdot_residual = _sys.solutionOlder();
  _u_dotdot_residual -= _sys.solutionOld();
  _u_dotdot_residual *= 1.0 / (_dt * _dt);

  // Computing udot residual
  // u_dot_residual = u_dot - (u - u_old)/2/dt = (u - u_older)/ 2/ dt - (u - u_old)/2/dt
  // u_dot_residual = (u_old - u_older)/2/dt
  _u_dot_residual = _sys.solutionOld();
  _u_dot_residual -= _sys.solutionOlder();
  _u_dot_residual *= 1.0 / (2.0 * _dt);
}

NumericVector<Number> &
CentralDifference::uDotDotResidual() const
{
  if (!_sys.solutionUDotDot())
    mooseError(
        "TimeIntegrator: Time derivative of solution (`u_dotdot`) is not stored. Please set "
        "uDotDotRequested() to true in FEProblemBase before requesting `u_dotdot_residual`.");
  if (_dt != 0)
    return _u_dotdot_residual;
  else
    return *_sys.solutionUDotDot();
}

NumericVector<Number> &
CentralDifference::uDotResidual() const
{
  if (!_sys.solutionUDot())
    mooseError("TimeIntegrator: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot_residual`.");
  if (_dt != 0)
    return _u_dot_residual;
  else
    return *_sys.solutionUDotDot();
}
