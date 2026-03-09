//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantsFromElasticityTensor.h"
#include "ElasticityTensorTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("SolidMechanicsApp", ConstantsFromElasticityTensor);
registerMooseObject("SolidMechanicsApp", ADConstantsFromElasticityTensor);

template <bool is_ad>
InputParameters
ConstantsFromElasticityTensorTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate elastic constants from an isotropic elasticity tensor");
  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");

  return params;
}

template <bool is_ad>
ConstantsFromElasticityTensorTempl<is_ad>::ConstantsFromElasticityTensorTempl(
    const InputParameters & parameters)
  : Material(parameters),
    GuaranteeConsumer(this),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _youngs_modulus(this->template declareGenericProperty<Real, is_ad>(_base_name +
                                                                       "youngs_modulus_from_tensor")),
    _poissons_ratio(this->template declareGenericProperty<Real, is_ad>(_base_name +
                                                                       "poissons_ratio_from_tensor")),
    _shear_modulus(this->template declareGenericProperty<Real, is_ad>(_base_name +
                                                                      "shear_modulus_from_tensor")),
    _bulk_modulus(this->template declareGenericProperty<Real, is_ad>(_base_name +
                                                                     "bulk_modulus_from_tensor")),
    _elasticity_tensor_name(getParam<MaterialPropertyName>("elasticity_tensor")),
    _elasticity_tensor(
        this->template getGenericMaterialPropertyByName<RankFourTensor, is_ad>(_base_name +
                                                                                _elasticity_tensor_name))
{
}

template <bool is_ad>
void
ConstantsFromElasticityTensorTempl<is_ad>::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_base_name + _elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("ConstantsFromElasticityTensor requires that the elasticity tensor be "
               "guaranteed isotropic");
}

template <bool is_ad>
void
ConstantsFromElasticityTensorTempl<is_ad>::computeQpProperties()
{
  const auto & elasticity_tensor = MetaPhysicL::raw_value(_elasticity_tensor[_qp]);

  _youngs_modulus[_qp] = ElasticityTensorTools::getIsotropicYoungsModulus(elasticity_tensor);
  _poissons_ratio[_qp] = ElasticityTensorTools::getIsotropicPoissonsRatio(elasticity_tensor);
  _shear_modulus[_qp] = ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);
  _bulk_modulus[_qp] = ElasticityTensorTools::getIsotropicBulkModulus(elasticity_tensor);
}

template class ConstantsFromElasticityTensorTempl<false>;
template class ConstantsFromElasticityTensorTempl<true>;
