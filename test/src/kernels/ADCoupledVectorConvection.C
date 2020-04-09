//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledVectorConvection.h"

registerMooseObject("MooseTestApp", ADCoupledVectorConvection);

InputParameters
ADCoupledVectorConvection::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addParam<bool>("use_grad_row", false, "Use first row of gradient.");
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection ADKernel");
  return params;
}

ADCoupledVectorConvection::ADCoupledVectorConvection(const InputParameters & parameters)
  : ADKernel(parameters),
    _use_grad(getParam<bool>("use_grad_row")),
    _velocity_vector(adCoupledVectorValue("velocity_vector")),
    _grad_velocity_vector(adCoupledVectorGradient("velocity_vector"))
{
}

ADReal
ADCoupledVectorConvection::computeQpResidual()
{
  if (_use_grad)
    return _test[_i][_qp] * _grad_velocity_vector[_qp].row(0) * _grad_u[_qp];
  else
    return _test[_i][_qp] * _velocity_vector[_qp] * _grad_u[_qp];
}
