//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUReaction.h"

registerMooseObject("MooseApp", KokkosReaction);

InputParameters
KokkosReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term with weak form $(\\psi_i, \\lambda u_h)$.");
  params.addParam<Real>(
      "rate", 1.0, "The $(\\lambda)$ multiplier, the relative amount consumed per unit time.");
  params.declareControllable("rate");
  return params;
}

KokkosReaction::KokkosReaction(const InputParameters & parameters)
  : Kernel(parameters), _rate(getParam<Real>("rate"))
{
}
