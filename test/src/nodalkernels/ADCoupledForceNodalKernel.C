//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledForceNodalKernel.h"

registerMooseObject("MooseTestApp", ADCoupledForceNodalKernel);

InputParameters
ADCoupledForceNodalKernel::validParams()
{
  InputParameters params = ADNodalKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>(
      "coef", 1.0, "Coefficient ($\\sigma$) multiplier for the coupled force term.");
  return params;
}

ADCoupledForceNodalKernel::ADCoupledForceNodalKernel(const InputParameters & parameters)
  : ADNodalKernel(parameters),
    _v_var(coupled("v")),
    _v(adCoupledValue("v")),
    _coef(getParam<Real>("coef"))
{
}

ADReal
ADCoupledForceNodalKernel::computeQpResidual()
{
  return -_coef * _v[_qp];
}
