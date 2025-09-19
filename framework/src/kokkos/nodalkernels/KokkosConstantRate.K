//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstantRate.h"

registerKokkosResidualObject("MooseApp", KokkosConstantRate);

InputParameters
KokkosConstantRate::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Computes residual or the rate in a simple ODE of du/dt = rate.");
  params.addRequiredParam<Real>("rate", "The constant rate in 'du/dt = rate'");
  params.declareControllable("rate");
  return params;
}

KokkosConstantRate::KokkosConstantRate(const InputParameters & parameters)
  : NodalKernel(parameters), _rate(getParam<Real>("rate"))
{
}
