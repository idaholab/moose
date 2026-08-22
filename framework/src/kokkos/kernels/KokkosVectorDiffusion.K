//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorDiffusion.h"

registerKokkosResidualObject("MooseApp", KokkosVectorDiffusion);

InputParameters
KokkosVectorDiffusion::validParams()
{
  InputParameters params = Moose::Kokkos::VectorKernelGrad::validParams();
  params.addClassDescription(
      "The Laplacian operator ($-\\nabla \\cdot \\nabla \\vec{u}$), with the weak "
      "form of $(\\nabla \\vec{\\phi_i}, \\nabla \\vec{u_h})$.");
  return params;
}

KokkosVectorDiffusion::KokkosVectorDiffusion(const InputParameters & parameters)
  : Moose::Kokkos::VectorKernelGrad(parameters)
{
}
