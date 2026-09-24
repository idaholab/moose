//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeSmallStrain.h"

registerKokkosMaterial("SolidMechanicsApp", KokkosComputeSmallStrain);
registerKokkosMaterial("SolidMechanicsApp", KokkosSymmetricComputeSmallStrain);

template <typename RankTwo>
InputParameters
KokkosComputeSmallStrainTempl<RankTwo>::validParams()
{
  InputParameters params = Moose::Kokkos::Material::validParams();
  params.addClassDescription("Compute a small strain from a vector displacement variable.");
  params.addRequiredCoupledVar(
      "displacements",
      "The vector variable carrying all of the displacement components, of a family such as "
      "LAGRANGE_VEC");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

template <typename RankTwo>
KokkosComputeSmallStrainTempl<RankTwo>::KokkosComputeSmallStrainTempl(
    const InputParameters & parameters)
  : Moose::Kokkos::Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _grad_disp(kokkosCoupledVectorGradient("displacements")),
    _total_strain(
        this->template declareKokkosPropertyByName<RankTwo>(_base_name + "total_strain")),
    _mechanical_strain(
        this->template declareKokkosPropertyByName<RankTwo>(_base_name + "mechanical_strain"))
{
  if (coupledComponents("displacements") != 1)
    paramError("displacements",
               "A single vector displacement variable is expected, carrying all of the "
               "displacement components, but ",
               coupledComponents("displacements"),
               " variables were supplied.");
}

template class KokkosComputeSmallStrainTempl<Moose::Kokkos::Real33>;
template class KokkosComputeSmallStrainTempl<Moose::Kokkos::Real6>;
