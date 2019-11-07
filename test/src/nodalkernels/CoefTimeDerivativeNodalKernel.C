//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefTimeDerivativeNodalKernel.h"

registerMooseObject("MooseTestApp", CoefTimeDerivativeNodalKernel);

InputParameters
CoefTimeDerivativeNodalKernel::validParams()
{
  InputParameters params = TimeDerivativeNodalKernel::validParams();
  params.addParam<Real>("coeff", 1, "The coefficient");
  return params;
}

CoefTimeDerivativeNodalKernel::CoefTimeDerivativeNodalKernel(const InputParameters & parameters)
  : TimeDerivativeNodalKernel(parameters), _coeff(getParam<Real>("coeff"))
{
}

Real
CoefTimeDerivativeNodalKernel::computeQpResidual()
{
  return _coeff * TimeDerivativeNodalKernel::computeQpResidual();
}

Real
CoefTimeDerivativeNodalKernel::computeQpJacobian()
{
  return _coeff * TimeDerivativeNodalKernel::computeQpJacobian();
}
