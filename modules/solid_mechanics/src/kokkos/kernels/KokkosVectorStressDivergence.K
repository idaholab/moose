//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorStressDivergence.h"

registerKokkosResidualObject("SolidMechanicsApp", KokkosVectorStressDivergence);

InputParameters
KokkosVectorStressDivergence::validParams()
{
  InputParameters params = Moose::Kokkos::VectorKernelGrad::validParams();
  params.addClassDescription(
      "Stress divergence for a vector displacement variable, with the weak form "
      "$(\\sigma, \\nabla \\vec{\\psi_i})$.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

KokkosVectorStressDivergence::KokkosVectorStressDivergence(const InputParameters & parameters)
  : Moose::Kokkos::VectorKernelGrad(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getKokkosMaterialPropertyByName<Moose::Kokkos::Real33>(_base_name + "stress")),
    _Jacobian_mult(
        getKokkosMaterialPropertyByName<Moose::Kokkos::Real3333>(_base_name + "Jacobian_mult"))
{
}
