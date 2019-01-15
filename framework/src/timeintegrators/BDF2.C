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

template <>
InputParameters
validParams<BDF2>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

BDF2::BDF2(const InputParameters & parameters)
  : TimeIntegrator(parameters), _weight(declareRestartableData<std::vector<Real>>("weight"))
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
    u_dot -= _solution_old;
    u_dot *= 1 / _dt;
    u_dot.close();

    _du_dot_du = 1.0 / _dt;
  }
  else
  {
    u_dot.zero();
    u_dot.add(_weight[0], *_solution);
    u_dot.add(_weight[1], _solution_old);
    u_dot.add(_weight[2], _solution_older);
    u_dot.scale(1. / _dt);
    u_dot.close();

    _du_dot_du = _weight[0] / _dt;
  }
}

void
BDF2::computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof)
{
  if (_t_step == 1)
  {
    const auto & local_old = _solution_old(dof);
    ad_u_dot -= local_old;
    ad_u_dot *= 1 / _dt;
  }
  else
  {
    ad_u_dot *= _weight[0];
    ad_u_dot += _weight[1] * _solution_old(dof);
    ad_u_dot += _weight[2] * _solution_older(dof);
    ad_u_dot *= 1. / _dt;
  }
}

void
BDF2::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}
