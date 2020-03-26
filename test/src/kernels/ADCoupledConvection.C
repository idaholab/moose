//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledConvection.h"

registerMooseObject("MooseTestApp", ADCoupledConvection);

InputParameters
ADCoupledConvection::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addParam<Real>("scale", 1, "Scaling coefficient");
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection ADKernel");
  return params;
}

ADCoupledConvection::ADCoupledConvection(const InputParameters & parameters)
  : ADKernel(parameters),
    _velocity_vector(adCoupledGradient("velocity_vector")),
    _scale(getParam<Real>("scale"))
{
}

ADReal
ADCoupledConvection::computeQpResidual()
{
  return _scale * _test[_i][_qp] * _velocity_vector[_qp] * _grad_u[_qp];
}
