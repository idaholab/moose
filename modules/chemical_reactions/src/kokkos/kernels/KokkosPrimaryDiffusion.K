//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPrimaryDiffusion.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosPrimaryDiffusion);

InputParameters
KokkosPrimaryDiffusion::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params.addClassDescription("Diffusion of primary species");
  return params;
}

KokkosPrimaryDiffusion::KokkosPrimaryDiffusion(const InputParameters & parameters)
  : KernelGrad(parameters), _diffusivity(getKokkosMaterialProperty<Real>("diffusivity"))
{
}
