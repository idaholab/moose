//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeDerivativeLM.h"

registerMooseObject("ScalarTransportApp", TimeDerivativeLM);

InputParameters
TimeDerivativeLM::validParams()
{
  auto params = LMTimeKernel::validParams();
  params.addClassDescription(
      "Adds a time derivative term to a Lagrange multiplier constrained primal equation");
  return params;
}

TimeDerivativeLM::TimeDerivativeLM(const InputParameters & parameters) : LMTimeKernel(parameters) {}

ADReal
TimeDerivativeLM::precomputeQpResidual()
{
  return _u_dot[_qp];
}
