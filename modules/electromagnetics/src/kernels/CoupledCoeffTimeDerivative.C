//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledCoeffTimeDerivative.h"

registerMooseObject("ElectromagneticsApp", CoupledCoeffTimeDerivative);

InputParameters
CoupledCoeffTimeDerivative::validParams()
{
  InputParameters params = CoupledTimeDerivative::validParams();
  params.addClassDescription("Coupled time derivative kernel for scalar variables, multiplied by a "
                             "user-specified coefficient.");
  params.addParam<Real>("coefficient", "User-provided coefficient for kernel");
  return params;
}

CoupledCoeffTimeDerivative::CoupledCoeffTimeDerivative(const InputParameters & parameters)
  : CoupledTimeDerivative(parameters), _coeff(getParam<Real>("coefficient"))
{
}

Real
CoupledCoeffTimeDerivative::computeQpResidual()
{
  return _coeff * CoupledTimeDerivative::computeQpResidual();
}

Real
CoupledCoeffTimeDerivative::computeQpJacobian()
{
  return _coeff * CoupledTimeDerivative::computeQpJacobian();
}
