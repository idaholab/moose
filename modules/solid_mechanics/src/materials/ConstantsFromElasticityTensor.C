//* This file is part of the MOOSE framework
//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantsFromElasticityTensor.h"
#include "RankFourTensor.h"
#include "ElasticityTensorTools.h"

registerMooseObject("SolidMechanicsApp", ConstantsFromElasticityTensor);

InputParameters
ConstantsFromElasticityTensor::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate elastic constants from an isotropic elasticity tensor");
  params.addParam<MaterialPropertyName>("elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");

  return params;
}

ConstantsFromElasticityTensor::ConstantsFromElasticityTensor(const InputParameters & parameters)
  : Material(parameters),
    GuaranteeConsumer(this),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _youngs_modulus(declareProperty<Real>(_base_name + "youngs_modulus_from_tensor")),
    _poissons_ratio(declareProperty<Real>(_base_name + "poissons_ratio_from_tensor")),
    _shear_modulus(declareProperty<Real>(_base_name + "shear_modulus_from_tensor")),
    _bulk_modulus(declareProperty<Real>(_base_name + "bulk_modulus_from_tensor")),
    _elasticity_tensor_name(getParam<MaterialPropertyName>(_base_name + "elasticity_tensor")),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name))
{
}

void
ConstantsFromElasticityTensor::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("ConstantsFromElasticityTensor requires that the elasticity tensor be "
               "guaranteed isotropic");
}


void
ConstantsFromElasticityTensor::computeQpProperties()
{
  _youngs_modulus[_qp] = ElasticityTensorTools::getIsotropicYoungsModulus(_elasticity_tensor[_qp]);
  _poissons_ratio[_qp] = ElasticityTensorTools::getIsotropicPoissonsRatio(_elasticity_tensor[_qp]);
  _shear_modulus[_qp] = ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
  _bulk_modulus[_qp] = ElasticityTensorTools::getIsotropicBulkModulus(_elasticity_tensor[_qp]);
}
