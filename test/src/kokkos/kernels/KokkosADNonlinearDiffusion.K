//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADNonlinearDiffusion.h"

registerKokkosResidualObject("MooseTestApp", KokkosADNonlinearDiffusion);

InputParameters
KokkosADNonlinearDiffusion::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("Diffusion with the solution-dependent conductivity 1 + u^2, "
                             "linearized by automatic differentiation at the quadrature point.");
  return params;
}

KokkosADNonlinearDiffusion::KokkosADNonlinearDiffusion(const InputParameters & parameters)
  : ADKernelGrad(parameters)
{
}
