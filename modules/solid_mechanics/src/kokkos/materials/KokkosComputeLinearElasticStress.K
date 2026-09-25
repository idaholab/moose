//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeLinearElasticStress.h"

#include "FEProblemBase.h"

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
    _Jacobian_mult(declareKokkosPropertyByName<Moose::Kokkos::Real3333>(
        _base_name + "Jacobian_mult", {}, Moose::Kokkos::PropertyConstantOption::SUBDOMAIN))
{
  _fe_problem.addKokkosMeshInitializationHook(
      std::bind(&KokkosComputeLinearElasticStress::checkElasticityTensorGranularity, this));
}

void
KokkosComputeLinearElasticStress::checkElasticityTensorGranularity()
{
  for (const auto block : blockIDs())
    if (_elasticity_tensor.constantOption(block) != Moose::Kokkos::PropertyConstantOption::SUBDOMAIN)
      mooseError("The tangent this material declares is stored once per subdomain, which is only "
                 "correct when the elasticity tensor it copies is too, but '",
                 _base_name,
                 "elasticity_tensor' is not constant on subdomains for block ",
                 block,
                 ". Leave the elasticity tensor material at its default constant_on = SUBDOMAIN.");
}
