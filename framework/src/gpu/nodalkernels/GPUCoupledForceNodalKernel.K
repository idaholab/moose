//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUCoupledForceNodalKernel.h"

registerMooseObject("MooseApp", GPUCoupledForceNodalKernel);

InputParameters
GPUCoupledForceNodalKernel::validParams()
{
  InputParameters params = GPUNodalKernel::validParams();
  params.addClassDescription("Adds a force proportional to the value of the coupled variable");
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");

  return params;
}

GPUCoupledForceNodalKernel::GPUCoupledForceNodalKernel(const InputParameters & parameters)
  : GPUNodalKernel(parameters),
    _v_var(coupled("v")),
    _v(GPUCoupledNodalValue("v")),
    _coef(getParam<Real>("coef"))
{
  if (_var.number() == _v_var)
    mooseError(
        "Coupled variable 'v' needs to be different from 'variable' with CoupledForceNodalKernel, "
        "consider using Reaction or somethig similar");
}
