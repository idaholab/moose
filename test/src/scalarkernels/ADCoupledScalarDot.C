//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledScalarDot.h"

registerMooseObject("MooseTestApp", ADCoupledScalarDot);

InputParameters
ADCoupledScalarDot::validParams()
{
  InputParameters params = ADScalarTimeKernel::validParams();
  params.addRequiredCoupledVar(
      "v", "The coupled variable whose time derivative we will apply to the residual");
  return params;
}

ADCoupledScalarDot::ADCoupledScalarDot(const InputParameters & parameters)
  : ADScalarKernel(parameters), _v_dot(adCoupledScalarDot("v"))
{
}

ADReal
ADCoupledScalarDot::computeQpResidual()
{
  return _v_dot[_i];
}
