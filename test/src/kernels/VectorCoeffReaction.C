//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoeffReaction.h"
#include "Function.h"

registerMooseObject("MooseTestApp", VectorCoeffReaction);

InputParameters
VectorCoeffReaction::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addRequiredParam<Real>("coefficient",
                                "The rate coefficient. Positive = sink. Negative = source.");
  return params;
}

VectorCoeffReaction::VectorCoeffReaction(const InputParameters & parameters)
  : VectorKernel(parameters), _coefficient(getParam<Real>("coefficient"))
{
}

Real
VectorCoeffReaction::computeQpResidual()
{
  return _test[_i][_qp] * _coefficient * _u[_qp];
}

Real
VectorCoeffReaction::computeQpJacobian()
{
  return _test[_i][_qp] * _coefficient * _phi[_j][_qp];
}
