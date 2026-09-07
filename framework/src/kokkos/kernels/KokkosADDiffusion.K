//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADDiffusion.h"

registerKokkosResidualObject("MooseApp", KokkosADDiffusion);

InputParameters
KokkosADDiffusion::validParams()
{
  auto params = ADKernelGrad::validParams();
  params.addClassDescription("Same as `KokkosDiffusion` in terms of physics/residual, but the "
                             "Jacobian is computed using forward automatic differentiation");
  return params;
}

KokkosADDiffusion::KokkosADDiffusion(const InputParameters & parameters) : ADKernelGrad(parameters) {}
