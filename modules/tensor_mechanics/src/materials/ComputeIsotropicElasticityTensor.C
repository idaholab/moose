/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeIsotropicElasticityTensor.h"
#include "Function.h"

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
  params.addParam<bool>("compute_compliance_tensor", false, "Compute the compliance tensor during construction of the elasticity tensor.  This can slightly reduce runtime when using complicated plasticity models, but adds to the memory used by MOOSE.");
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
    _youngs_modulus(_youngs_modulus_set ? getParam<Real>("youngs_modulus") : -1),
    _compute_compliance_tensor(getParam<bool>("compute_compliance_tensor")),
    _compliance_tensor(_compute_compliance_tensor ? &declareProperty<RankFourTensor>(_base_name + "compliance_tensor") : nullptr)
{
  unsigned int num_elastic_constants = _bulk_modulus_set + _lambda_set + _poissons_ratio_set +
                                       _shear_modulus_set + _youngs_modulus_set;

  if (num_elastic_constants != 2)
    mooseError("Exactly two elastic constants must be defined for material '" + name() + "'.");

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
  _Eijkl.fillFromInputVector(iso_const, RankFourTensor::symmetric_isotropic);

  if (_compute_compliance_tensor)
    _Cijkl = _Eijkl.invSymm();
  else
    _Cijkl = RankFourTensor();
}

void
ComputeIsotropicElasticityTensor::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Eijkl;
  if (_compute_compliance_tensor)
  {
    (*_compliance_tensor)[_qp] = _Cijkl;
    if (_prefactor_function)
      (*_compliance_tensor)[_qp] /= _prefactor_function->value(_t, _q_point[_qp]);
  }
}
