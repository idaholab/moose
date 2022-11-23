//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForceLM.h"

registerMooseObject("ScalarTransportApp", CoupledForceLM);

InputParameters
CoupledForceLM::validParams()
{
  auto params = LMKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription(
      "Adds a coupled force term to a Lagrange multiplier constrained primal equation");
  return params;
}

CoupledForceLM::CoupledForceLM(const InputParameters & parameters)
  : LMKernel(parameters),
    _v_var(coupled("v")),
    _v(adCoupledValue("v")),
    _coef(getParam<Real>("coef"))
{
}

ADReal
CoupledForceLM::precomputeQpResidual()
{
  return -_coef * _v[_qp];
}
