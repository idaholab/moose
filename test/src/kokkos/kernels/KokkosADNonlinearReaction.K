//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADNonlinearReaction.h"

registerKokkosResidualObject("MooseTestApp", KokkosADNonlinearReaction);

InputParameters
KokkosADNonlinearReaction::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("The reaction-like term u^2 + grad(u) . grad(u), linearized by "
                             "automatic differentiation at the quadrature point.");
  return params;
}

KokkosADNonlinearReaction::KokkosADNonlinearReaction(const InputParameters & parameters)
  : ADKernelValue(parameters)
{
}
