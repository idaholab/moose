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

InputParameters
CentralDifference::validParams()
{
  InputParameters params = ActuallyExplicitEuler::validParams();

  params.addClassDescription("Implementation of explicit, Central Difference integration without "
                             "invoking any of the nonlinear solver");

  return params;
}

CentralDifference::CentralDifference(const InputParameters & parameters)
  : ActuallyExplicitEuler(parameters),
    _du_dotdot_du(_sys.duDotDotDu()),
    _solution_older(_sys.solutionState(2)),
    _solution_old_old_old(_sys.solutionState(3))
{
  _is_explicit = true;
  if (_solve_type == LUMPED)
    _is_lumped = true;

  _fe_problem.setUDotOldRequested(true);
  _fe_problem.setUDotDotRequested(true);
  _fe_problem.setUDotDotOldRequested(true);
}

void
CentralDifference::computeADTimeDerivatives(DualReal & ad_u_dot,
                                            const dof_id_type & dof,
                                            DualReal & ad_u_dotdot) const
{
  computeTimeDerivativeHelper(
      ad_u_dot, ad_u_dotdot, _solution_old(dof), _solution_older(dof), _solution_old_old_old(dof));
}

void
CentralDifference::initialSetup()
{
  ActuallyExplicitEuler::initialSetup();

  // _nl here so that we don't create this vector in the aux system time integrator
  _nl.disassociateVectorFromTag(*_nl.solutionUDot(), _u_dot_factor_tag);
  _nl.addVector(_u_dot_factor_tag, true, GHOSTED);
  _nl.disassociateVectorFromTag(*_nl.solutionUDotDot(), _u_dotdot_factor_tag);
  _nl.addVector(_u_dotdot_factor_tag, true, GHOSTED);
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
  auto & u_dot = *_sys.solutionUDot();
  auto & u_dotdot = *_sys.solutionUDotDot();

  // Computing derivatives
  computeTimeDerivativeHelper(
      u_dot, u_dotdot, _solution_old, _solution_older, _solution_old_old_old);

  // make sure _u_dotdot and _u_dot are in good state
  u_dotdot.close();
  u_dot.close();

  // used for Jacobian calculations
  _du_dot_du = 1.0 / (2 * _dt);
  _du_dotdot_du = 1.0 / (_dt * _dt);

  // Computing udotdot "factor"
  // u_dotdot_factor = u_dotdot - (u - u_old)/dt^2 = (u - 2* u_old + u_older - u + u_old) / dt^2
  // u_dotdot_factor = (u_older - u_old)/dt^2
  if (_sys.hasVector(_u_dotdot_factor_tag)) // so that we don't do this in the aux system
  {
    auto & u_dotdot_factor = _sys.getVector(_u_dotdot_factor_tag);
    u_dotdot_factor = _sys.solutionOlder();
    u_dotdot_factor -= _sys.solutionOld();
    u_dotdot_factor *= 1.0 / (_dt * _dt);
    u_dotdot_factor.close();
  }

  // Computing udot "factor"
  // u_dot_factor = u_dot - (u - u_old)/2/dt = (u - u_older)/ 2/ dt - (u - u_old)/2/dt
  // u_dot_factor = (u_old - u_older)/2/dt
  if (_sys.hasVector(_u_dot_factor_tag)) // so that we don't do this in the aux system
  {
    auto & u_dot_factor = _sys.getVector(_u_dot_factor_tag);
    u_dot_factor = _sys.solutionOld();
    u_dot_factor -= _sys.solutionOlder();
    u_dot_factor *= 1.0 / (2.0 * _dt);
    u_dot_factor.close();
  }
}
