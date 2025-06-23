//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUConstantRate.h"

registerMooseObject("MooseApp", GPUConstantRate);

InputParameters
GPUConstantRate::validParams()
{
  InputParameters params = GPUNodalKernel::validParams();
  params.addClassDescription("Computes residual or the rate in a simple ODE of du/dt = rate.");
  params.addRequiredParam<Real>("rate", "The constant rate in 'du/dt = rate'");
  params.declareControllable("rate");
  return params;
}

GPUConstantRate::GPUConstantRate(const InputParameters & parameters)
  : GPUNodalKernel(parameters), _rate(getParam<Real>("rate"))
{
}
