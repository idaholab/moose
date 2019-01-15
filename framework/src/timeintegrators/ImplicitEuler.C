//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitEuler.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", ImplicitEuler);

template <>
InputParameters
validParams<ImplicitEuler>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

ImplicitEuler::ImplicitEuler(const InputParameters & parameters) : TimeIntegrator(parameters) {}

ImplicitEuler::~ImplicitEuler() {}

void
ImplicitEuler::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("ImplicitEuler: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  u_dot -= _solution_old;
  u_dot *= 1 / _dt;
  u_dot.close();

  _du_dot_du = 1.0 / _dt;
}

void
ImplicitEuler::computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof)
{
  ad_u_dot -= _solution_old(dof);
  ad_u_dot *= 1 / _dt;
}

void
ImplicitEuler::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}
