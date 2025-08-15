//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericGradientComponent.h"

registerMooseObject("PhaseFieldApp", GenericGradientComponent);

InputParameters
GenericGradientComponent::validParams()
{
  InputParameters params = GradientComponent::validParams();
  params.addClassDescription(
      "Set the kernel variable to a specified component of the gradient of a coupled variable with a scaling factor");
  params.addParam<Real>("factor",
                                  "factor to be multiplied with the gradient component optional"); 
  return params;
}

GenericGradientComponent::GenericGradientComponent(const InputParameters & parameters)
  : GradientComponent(parameters),
    _factor(getParam<Real>("factor"))

{
}

Real
GenericGradientComponent::computeQpResidual()
{
  return (_u[_qp] + (_factor) * _grad_v[_qp](_component)) * _test[_i][_qp]; 
} 

Real
GenericGradientComponent::computeQpJacobian()
{
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
GenericGradientComponent::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
  return (_factor) * _grad_phi[_j][_qp](_component) * _test[_i][_qp]; // + _grad_phi instead of negative
  return 0.0;
}
