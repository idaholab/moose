//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosArrayDiffusion.h"

registerKokkosResidualObject("MooseApp", KokkosArrayDiffusion);

InputParameters
KokkosArrayDiffusion::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "diffusion_coefficient",
      "The diffusivity, supplied as a scalar, one-dimensional, or two-dimensional Kokkos "
      "material property.");
  params.addClassDescription(
      "The array Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak form of "
      "$(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

KokkosArrayDiffusion::KokkosArrayDiffusion(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _has_d(hasKokkosMaterialProperty<Real>("diffusion_coefficient")),
    _has_d_1d(hasKokkosMaterialProperty<Real, 1>("diffusion_coefficient")),
    _has_d_2d(hasKokkosMaterialProperty<Real, 2>("diffusion_coefficient")),
    _d(_has_d ? getKokkosMaterialProperty<Real>("diffusion_coefficient")
              : Moose::Kokkos::MaterialProperty<Real>()),
    _d_1d(_has_d_1d ? getKokkosMaterialProperty<Real, 1>("diffusion_coefficient")
                    : Moose::Kokkos::MaterialProperty<Real, 1>()),
    _d_2d(_has_d_2d ? getKokkosMaterialProperty<Real, 2>("diffusion_coefficient")
                    : Moose::Kokkos::MaterialProperty<Real, 2>())
{
  if (!_has_d && !_has_d_1d && !_has_d_2d)
    paramError("diffusion_coefficient",
               "The property is not a scalar, one-dimensional, or two-dimensional Kokkos "
               "material property.");
}
