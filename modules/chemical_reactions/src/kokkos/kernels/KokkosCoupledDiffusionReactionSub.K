//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCoupledDiffusionReactionSub.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosCoupledDiffusionReactionSub);

InputParameters
KokkosCoupledDiffusionReactionSub::validParams()
{
  InputParameters params = KokkosEquilibriumReactionFlux<KernelGrad>::validParams();
  params.addClassDescription("Diffusion of equilibrium species");
  return params;
}

KokkosCoupledDiffusionReactionSub::KokkosCoupledDiffusionReactionSub(
    const InputParameters & parameters)
  : KokkosEquilibriumReactionFlux<KernelGrad>(parameters),
    _diffusivity(getKokkosMaterialProperty<Real>("diffusivity"))
{
}
