/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeIsotropicElasticityTensor.h"

template <>
InputParameters
validParams<ComputeIsotropicElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an isotropic elasticity tensor.");
  params.addParam<Real>("bulk_modulus", "The bulk modulus for the material.");
  params.addParam<Real>("lambda", "Lame's first constant for the material.");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addParam<Real>("shear_modulus", "The shear modulus of the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  return params;
}

ComputeIsotropicElasticityTensor::ComputeIsotropicElasticityTensor(
    const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _bulk_modulus_set(parameters.isParamValid("bulk_modulus")),
    _lambda_set(parameters.isParamValid("lambda")),
    _poissons_ratio_set(parameters.isParamValid("poissons_ratio")),
    _shear_modulus_set(parameters.isParamValid("shear_modulus")),
    _youngs_modulus_set(parameters.isParamValid("youngs_modulus")),
    _bulk_modulus(_bulk_modulus_set ? getParam<Real>("bulk_modulus") : -1),
    _lambda(_lambda_set ? getParam<Real>("lambda") : -1),
    _poissons_ratio(_poissons_ratio_set ? getParam<Real>("poissons_ratio") : -1),
    _shear_modulus(_shear_modulus_set ? getParam<Real>("shear_modulus") : -1),
    _youngs_modulus(_youngs_modulus_set ? getParam<Real>("youngs_modulus") : -1)
{
  std::vector<Real> iso_const(2);

  if (_lambda_set && _shear_modulus_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = _shear_modulus;
  }
  else if (_youngs_modulus_set && _poissons_ratio_set)
  {
    iso_const[0] = _youngs_modulus * _poissons_ratio /
                   ((1.0 + _poissons_ratio) * (1.0 - 2.0 * _poissons_ratio));
    iso_const[1] = _youngs_modulus / (2.0 * (1.0 + _poissons_ratio));
  }
  else if (_shear_modulus_set && _bulk_modulus_set)
  {
    iso_const[0] = _bulk_modulus - 2.0 / 3.0 * _shear_modulus;
    iso_const[1] = _shear_modulus;
  }
  else if (_poissons_ratio_set && _bulk_modulus_set)
  {
    iso_const[0] = 3.0 * _bulk_modulus * _poissons_ratio / (1.0 + _poissons_ratio);
    iso_const[1] =
        3.0 * _bulk_modulus * (1.0 - 2.0 * _poissons_ratio) / (2.0 * (1.0 + _poissons_ratio));
  }
  else if (_lambda_set && _bulk_modulus_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = 3.0 * (_bulk_modulus - _lambda) / 2.0;
  }
  else if (_shear_modulus_set && _youngs_modulus_set)
  {
    iso_const[0] = _shear_modulus * (_youngs_modulus - 2.0 * _shear_modulus) /
                   (3.0 * _shear_modulus - _youngs_modulus);
    iso_const[1] = _shear_modulus;
  }
  else if (_shear_modulus_set && _poissons_ratio_set)
  {
    iso_const[0] = 2.0 * _shear_modulus * _poissons_ratio / (1.0 - 2.0 * _poissons_ratio);
    iso_const[1] = _shear_modulus;
  }
  else if (_youngs_modulus_set && _bulk_modulus_set)
  {
    iso_const[0] = 3.0 * _bulk_modulus * (3.0 * _bulk_modulus - _youngs_modulus) /
                   (9.0 * _bulk_modulus - _youngs_modulus);
    iso_const[1] = 3.0 * _bulk_modulus * _youngs_modulus / (9.0 * _bulk_modulus - _youngs_modulus);
  }
  else if (_lambda_set && _poissons_ratio_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = _lambda * (1.0 - 2.0 * _poissons_ratio) / (2.0 * _poissons_ratio);
  }
  else if (_lambda_set && _youngs_modulus_set)
  {
    iso_const[0] = _lambda;
    iso_const[1] = (_youngs_modulus - 3.0 * _lambda +
                    std::sqrt(_youngs_modulus * _youngs_modulus + 9.0 * _lambda * _lambda +
                              2.0 * _youngs_modulus * _lambda)) /
                   4.0;
  }
  else
    mooseError("Incorrect combination of elastic properties in ComputeIsotropicElasticityTensor.");

  // Fill elasticity tensor
  _Cijkl.fillFromInputVector(iso_const, RankFourTensor::symmetric_isotropic);
}

void
ComputeIsotropicElasticityTensor::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;
}
