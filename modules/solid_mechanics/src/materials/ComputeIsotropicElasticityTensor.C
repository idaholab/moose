//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIsotropicElasticityTensor.h"
#include "ElasticityTensorTools.h"

registerMooseObject("SolidMechanicsApp", ComputeIsotropicElasticityTensor);
registerMooseObject("SolidMechanicsApp", ADComputeIsotropicElasticityTensor);
registerMooseObject("SolidMechanicsApp", SymmetricIsotropicElasticityTensor);
registerMooseObject("SolidMechanicsApp", ADSymmetricIsotropicElasticityTensor);

template <bool is_ad, typename T>
InputParameters
ComputeIsotropicElasticityTensorTempl<is_ad, T>::validParams()
{
  InputParameters params = ComputeElasticityTensorBase::validParams();
  params.addClassDescription("Compute a constant isotropic elasticity tensor.");
  params.addParam<Real>("bulk_modulus", -1, "The bulk modulus for the material.");
  params.addParam<Real>("lambda", -1, "Lame's first constant for the material.");
  params.addParam<Real>("poissons_ratio", -1, "Poisson's ratio for the material.");
  params.addParam<Real>("shear_modulus", -1, "The shear modulus of the material.");
  params.addParam<Real>("youngs_modulus", -1, "Young's modulus of the material.");
  params.declareControllable("bulk_modulus lambda poissons_ratio shear_modulus youngs_modulus");
  return params;
}

template <bool is_ad, typename T>
ComputeIsotropicElasticityTensorTempl<is_ad, T>::ComputeIsotropicElasticityTensorTempl(
    const InputParameters & parameters)
  : ComputeElasticityTensorBaseTempl<is_ad, T>(parameters),
    _bulk_modulus_set(parameters.isParamSetByUser("bulk_modulus")),
    _lambda_set(parameters.isParamSetByUser("lambda")),
    _poissons_ratio_set(parameters.isParamSetByUser("poissons_ratio")),
    _shear_modulus_set(parameters.isParamSetByUser("shear_modulus")),
    _youngs_modulus_set(parameters.isParamSetByUser("youngs_modulus")),
    _bulk_modulus(this->template getParam<Real>("bulk_modulus")),
    _lambda(this->template getParam<Real>("lambda")),
    _poissons_ratio(this->template getParam<Real>("poissons_ratio")),
    _shear_modulus(this->template getParam<Real>("shear_modulus")),
    _youngs_modulus(this->template getParam<Real>("youngs_modulus")),
    _effective_stiffness_local(parameters.isParamValid("effective_stiffness_local"))
{
  unsigned int num_elastic_constants = _bulk_modulus_set + _lambda_set + _poissons_ratio_set +
                                       _shear_modulus_set + _youngs_modulus_set;
  if (num_elastic_constants != 2)
    mooseError("Exactly two elastic constants must be defined for material '" + name() + "'.");

  // all tensors created by this class are always isotropic
  issueGuarantee(_elasticity_tensor_name, Guarantee::ISOTROPIC);
  issueGuarantee("effective_stiffness", Guarantee::ISOTROPIC);
  if (!isParamValid("elasticity_tensor_prefactor"))
    issueGuarantee(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);

  if (_bulk_modulus_set && _bulk_modulus <= 0.0)
    mooseError("Bulk modulus must be positive in material '" + name() + "'.");

  if (_poissons_ratio_set && (_poissons_ratio <= -1.0 || _poissons_ratio >= 0.5))
    mooseError("Poissons ratio must be greater than -1 and less than 0.5 in "
               "material '" +
               name() + "'.");

  if (_shear_modulus_set && _shear_modulus < 0.0)
    mooseError("Shear modulus must not be negative in material '" + name() + "'.");

  if (_youngs_modulus_set && _youngs_modulus <= 0.0)
    mooseError("Youngs modulus must be positive in material '" + name() + "'.");
}

template <bool is_ad, typename T>
void
ComputeIsotropicElasticityTensorTempl<is_ad, T>::residualSetup()
{
  ElasticityTensorTools::IsotropicElasticInput input;

  if (_bulk_modulus_set)
    input.bulk_modulus = _bulk_modulus;
  if (_lambda_set)
    input.lambda = _lambda;
  if (_poissons_ratio_set)
    input.poissons_ratio = _poissons_ratio;
  if (_shear_modulus_set)
    input.shear_modulus = _shear_modulus;
  if (_youngs_modulus_set)
    input.youngs_modulus = _youngs_modulus;

  const auto constants = ElasticityTensorTools::isotropicElasticConstants(input);

  _effective_stiffness_local = constants.effective_stiffness;

  // Fill elasticity tensor
  _Cijkl.fillFromInputVector({constants.lambda, constants.shear_modulus}, T::symmetric_isotropic);
}

template <bool is_ad, typename T>
void
ComputeIsotropicElasticityTensorTempl<is_ad, T>::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;

  // Assign effective stiffness at a given quad point
  _effective_stiffness[_qp] = _effective_stiffness_local;
}

template class ComputeIsotropicElasticityTensorTempl<false, RankFourTensor>;
template class ComputeIsotropicElasticityTensorTempl<true, RankFourTensor>;
template class ComputeIsotropicElasticityTensorTempl<false, SymmetricRankFourTensor>;
template class ComputeIsotropicElasticityTensorTempl<true, SymmetricRankFourTensor>;
