//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIsotropicElasticityTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeIsotropicElasticityTensor);
registerMooseObject("TensorMechanicsApp", ADComputeIsotropicElasticityTensor);
registerMooseObject("TensorMechanicsApp", SymmetricIsotropicElasticityTensor);
registerMooseObject("TensorMechanicsApp", ADSymmetricIsotropicElasticityTensor);

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
  std::vector<Real> iso_const(2);
  Real elas_mod;
  Real poiss_rat;

  if (_youngs_modulus_set && _poissons_ratio_set)
  {
    _Cijkl.fillSymmetricIsotropicEandNu(_youngs_modulus, _poissons_ratio);
    _effective_stiffness_local =
        std::max(std::sqrt((_youngs_modulus * (1 - _poissons_ratio)) /
                           ((1 + _poissons_ratio) * (1 - 2 * _poissons_ratio))),
                 std::sqrt(_youngs_modulus / (2 * (1 + _poissons_ratio))));
    return;
  }

  if (_lambda_set && _shear_modulus_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = _shear_modulus;
    elas_mod = (_shear_modulus * (3 * _lambda + 2 * _shear_modulus)) / (_lambda + _shear_modulus);
    poiss_rat = _lambda / (2 * (_lambda + _shear_modulus));
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(_shear_modulus));
  }
  else if (_shear_modulus_set && _bulk_modulus_set)
  {
    iso_const[0] = _bulk_modulus - 2.0 / 3.0 * _shear_modulus;
    iso_const[1] = _shear_modulus;
    elas_mod = (9 * _bulk_modulus * _shear_modulus) / (3 * _bulk_modulus + _shear_modulus);
    poiss_rat =
        (3 * _bulk_modulus - 2 * _shear_modulus) / (2 * (3 * _bulk_modulus + _shear_modulus));
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(_shear_modulus));
  }
  else if (_poissons_ratio_set && _bulk_modulus_set)
  {
    iso_const[0] = 3.0 * _bulk_modulus * _poissons_ratio / (1.0 + _poissons_ratio);
    iso_const[1] =
        3.0 * _bulk_modulus * (1.0 - 2.0 * _poissons_ratio) / (2.0 * (1.0 + _poissons_ratio));
    elas_mod = 3 * _bulk_modulus * (1 - 2 * _poissons_ratio);
    poiss_rat = _poissons_ratio;
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (_lambda_set && _bulk_modulus_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = 3.0 * (_bulk_modulus - _lambda) / 2.0;
    elas_mod = (9 * _bulk_modulus * (_bulk_modulus - _lambda)) / (3 * _bulk_modulus - _lambda);
    poiss_rat = (_lambda) / ((3 * _bulk_modulus - _lambda));
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (_shear_modulus_set && _youngs_modulus_set)
  {
    iso_const[0] = _shear_modulus * (_youngs_modulus - 2.0 * _shear_modulus) /
                   (3.0 * _shear_modulus - _youngs_modulus);
    iso_const[1] = _shear_modulus;
    elas_mod = _youngs_modulus;
    poiss_rat = (_youngs_modulus - 2 * _shear_modulus) / (2 * _shear_modulus);
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (_shear_modulus_set && _poissons_ratio_set)
  {
    iso_const[0] = 2.0 * _shear_modulus * _poissons_ratio / (1.0 - 2.0 * _poissons_ratio);
    iso_const[1] = _shear_modulus;
    elas_mod = (2 * _shear_modulus * (1 + _poissons_ratio));
    poiss_rat = (_poissons_ratio);
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (_youngs_modulus_set && _bulk_modulus_set)
  {
    iso_const[0] = 3.0 * _bulk_modulus * (3.0 * _bulk_modulus - _youngs_modulus) /
                   (9.0 * _bulk_modulus - _youngs_modulus);
    iso_const[1] = 3.0 * _bulk_modulus * _youngs_modulus / (9.0 * _bulk_modulus - _youngs_modulus);
    elas_mod = (_youngs_modulus);
    poiss_rat = (3 * _bulk_modulus - _youngs_modulus) / (6 * _bulk_modulus);
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (_lambda_set && _poissons_ratio_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = _lambda * (1.0 - 2.0 * _poissons_ratio) / (2.0 * _poissons_ratio);
    elas_mod = (_lambda * (1 + _poissons_ratio) * (1 - 2 * _poissons_ratio)) / (_poissons_ratio);
    poiss_rat = (_poissons_ratio);
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (_lambda_set && _youngs_modulus_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = (_youngs_modulus - 3.0 * _lambda +
                    std::sqrt(_youngs_modulus * _youngs_modulus + 9.0 * _lambda * _lambda +
                              2.0 * _youngs_modulus * _lambda)) /
                   4.0;
    elas_mod = (_youngs_modulus);
    poiss_rat = (2 * _lambda) / (_youngs_modulus + _lambda +
                                 std::sqrt(std::pow(_youngs_modulus, 2) + 9 * std::pow(_lambda, 2) +
                                           2 * _youngs_modulus * _lambda));
    _effective_stiffness_local =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else
    mooseError("Incorrect combination of elastic properties in ComputeIsotropicElasticityTensor.");

  // Fill elasticity tensor
  _Cijkl.fillFromInputVector(iso_const, T::symmetric_isotropic);
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
