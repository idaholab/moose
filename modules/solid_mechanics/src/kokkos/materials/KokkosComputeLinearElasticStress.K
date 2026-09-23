//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeLinearElasticStress.h"

registerKokkosMaterial("SolidMechanicsApp", KokkosComputeLinearElasticStress);

InputParameters
KokkosComputeLinearElasticStress::validParams()
{
  InputParameters params = Moose::Kokkos::Material::validParams();
  params.addClassDescription("Compute stress using elasticity for small strains.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

KokkosComputeLinearElasticStress::KokkosComputeLinearElasticStress(
    const InputParameters & parameters)
  : Moose::Kokkos::Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(
        getKokkosMaterialPropertyByName<Moose::Kokkos::Real3333>(_base_name + "elasticity_tensor")),
    _mechanical_strain(
        getKokkosMaterialPropertyByName<Moose::Kokkos::Real33>(_base_name + "mechanical_strain")),
    _stress(declareKokkosPropertyByName<Moose::Kokkos::Real33>(_base_name + "stress")),
    _Jacobian_mult(
        declareKokkosPropertyByName<Moose::Kokkos::Real3333>(_base_name + "Jacobian_mult"))
{
}
