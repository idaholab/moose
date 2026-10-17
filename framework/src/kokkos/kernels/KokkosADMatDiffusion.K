//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADMatDiffusion.h"

registerKokkosADResidualObject("MooseApp", KokkosADMatDiffusion);

InputParameters
KokkosADMatDiffusion::validParams()
{
  auto params = ADKernel::validParams();
  params.addClassDescription(
      "Diffusion with a diffusivity given by a pointwise AD material property, with the Jacobian "
      "computed by pointwise automatic differentiation");
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity", "The pointwise AD material property giving the diffusivity");
  return params;
}

KokkosADMatDiffusion::KokkosADMatDiffusion(const InputParameters & parameters)
  : ADKernel(parameters),
    _grad_u(adGradient()),
    _diffusivity(getKokkosADMaterialProperty<Real>("diffusivity"))
{
}
