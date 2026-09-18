//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearElasticity.h"

registerKokkosResidualObject("SolidMechanicsApp", KokkosLinearElasticity);

InputParameters
KokkosLinearElasticity::validParams()
{
  InputParameters params = Moose::Kokkos::VectorKernelGrad::validParams();
  params.addClassDescription(
      "Isotropic linear elasticity for a vector displacement variable, with the weak form "
      "$(\\sigma(\\vec{u}), \\nabla \\vec{\\psi_i})$ where $\\sigma = \\lambda "
      "\\mathrm{tr}(\\epsilon) I + 2 \\mu \\epsilon$.");
  params.addRequiredParam<MaterialPropertyName>("lambda", "The first Lame parameter");
  params.addRequiredParam<MaterialPropertyName>(
      "mu", "The second Lame parameter, also known as the shear modulus");
  return params;
}

KokkosLinearElasticity::KokkosLinearElasticity(const InputParameters & parameters)
  : Moose::Kokkos::VectorKernelGrad(parameters),
    _lambda(getKokkosMaterialProperty<Real>("lambda")),
    _mu(getKokkosMaterialProperty<Real>("mu"))
{
}
