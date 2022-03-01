//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BDF2.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", BDF2);

InputParameters
BDF2::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription(
      "Second order backward differentiation formula time integration scheme.");
  return params;
}

BDF2::BDF2(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _weight(declareRestartableData<std::vector<Real>>("weight")),
    _solution_older(_sys.solutionState(2))
{
  _weight.resize(3);
}

void
BDF2::preStep()
{
  if (_t_step > 1)
  {
    Real sum = _dt + _dt_old;
    _weight[0] = 1. + _dt / sum;
    _weight[1] = -sum / _dt_old;
    _weight[2] = _dt * _dt / _dt_old / sum;
  }
}

void
BDF2::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("BDF2: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  if (_t_step == 1)
  {
    u_dot = *_solution;
    _du_dot_du = 1. / _dt;
  }
  else
  {
    u_dot.zero();
    _du_dot_du = _weight[0] / _dt;
  }
  computeTimeDerivativeHelper(u_dot, *_solution, _solution_old, _solution_older);
  u_dot.close();
}

void
BDF2::computeADTimeDerivatives(DualReal & ad_u_dot,
                               const dof_id_type & dof,
                               DualReal & /*ad_u_dotdot*/) const
{
  auto ad_sln = ad_u_dot;
  if (_t_step != 1)
    ad_u_dot = 0;
  computeTimeDerivativeHelper(ad_u_dot, ad_sln, _solution_old(dof), _solution_older(dof));
}

void
BDF2::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}
