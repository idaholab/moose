//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNonlinearDiffusionGrad.h"

registerKokkosResidualObject("KokkosTrainingApp", KokkosNonlinearDiffusionGrad);

InputParameters
KokkosNonlinearDiffusionGrad::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params.addClassDescription(
      "Nonlinear diffusion with the conductivity k(u) = k0 (1 + beta u), using KernelGrad.");
  params.addParam<Real>("k0", 1.0, "Conductivity at u = 0");
  params.addParam<Real>("beta", 0.0, "Linear coefficient of the conductivity in u");
  return params;
}

KokkosNonlinearDiffusionGrad::KokkosNonlinearDiffusionGrad(const InputParameters & parameters)
  : KernelGrad(parameters), _k0(getParam<Real>("k0")), _beta(getParam<Real>("beta"))
{
}
