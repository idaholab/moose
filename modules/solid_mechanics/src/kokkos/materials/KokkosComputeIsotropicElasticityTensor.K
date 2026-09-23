//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeIsotropicElasticityTensor.h"

#include "ElasticityTensorTools.h"
#include "RankFourTensor.h"

registerKokkosMaterial("SolidMechanicsApp", KokkosComputeIsotropicElasticityTensor);

InputParameters
KokkosComputeIsotropicElasticityTensor::validParams()
{
  InputParameters params = Moose::Kokkos::Material::validParams();
  params.addClassDescription("Compute a constant isotropic elasticity tensor.");
  params.addParam<Real>("bulk_modulus", -1, "The bulk modulus for the material.");
  params.addParam<Real>("lambda", -1, "Lame's first constant for the material.");
  params.addParam<Real>("poissons_ratio", -1, "Poisson's ratio for the material.");
  params.addParam<Real>("shear_modulus", -1, "The shear modulus of the material.");
  params.addParam<Real>("youngs_modulus", -1, "Young's modulus of the material.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");

  // The tensor varies neither in space nor in time, so a single value per subdomain is stored
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";

  return params;
}

KokkosComputeIsotropicElasticityTensor::KokkosComputeIsotropicElasticityTensor(
    const InputParameters & parameters)
  : Moose::Kokkos::Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(
        declareKokkosPropertyByName<Moose::Kokkos::Real3333>(_base_name + "elasticity_tensor"))
{
  ElasticityTensorTools::IsotropicElasticInput input;

  if (isParamSetByUser("bulk_modulus"))
    input.bulk_modulus = getParam<Real>("bulk_modulus");
  if (isParamSetByUser("lambda"))
    input.lambda = getParam<Real>("lambda");
  if (isParamSetByUser("poissons_ratio"))
    input.poissons_ratio = getParam<Real>("poissons_ratio");
  if (isParamSetByUser("shear_modulus"))
    input.shear_modulus = getParam<Real>("shear_modulus");
  if (isParamSetByUser("youngs_modulus"))
    input.youngs_modulus = getParam<Real>("youngs_modulus");

  const unsigned int num_set = input.bulk_modulus.has_value() + input.lambda.has_value() +
                               input.poissons_ratio.has_value() + input.shear_modulus.has_value() +
                               input.youngs_modulus.has_value();
  if (num_set != 2)
    mooseError("Exactly two elastic constants must be defined.");

  if (input.bulk_modulus && *input.bulk_modulus <= 0.0)
    paramError("bulk_modulus", "Bulk modulus must be positive.");
  if (input.poissons_ratio && (*input.poissons_ratio <= -1.0 || *input.poissons_ratio >= 0.5))
    paramError("poissons_ratio", "Poissons ratio must be greater than -1 and less than 0.5.");
  if (input.shear_modulus && *input.shear_modulus < 0.0)
    paramError("shear_modulus", "Shear modulus must not be negative.");
  if (input.youngs_modulus && *input.youngs_modulus <= 0.0)
    paramError("youngs_modulus", "Youngs modulus must be positive.");

  const auto constants = ElasticityTensorTools::isotropicElasticConstants(input);

  RankFourTensor Cijkl;
  Cijkl.fillFromInputVector({constants.lambda, constants.shear_modulus},
                            RankFourTensor::symmetric_isotropic);

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
          _Cijkl(i, j, k, l) = Cijkl(i, j, k, l);
}
