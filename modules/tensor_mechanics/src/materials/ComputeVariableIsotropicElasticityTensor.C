/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVariableIsotropicElasticityTensor.h"

template<>
InputParameters validParams<ComputeVariableIsotropicElasticityTensor>()
{
  InputParameters params = validParams<ComputeIsotropicElasticityTensor>();
  params.addClassDescription("Compute an isotropic elasticity tensor for elastic constants that change as a function of temperature");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addParam<bool>("store_old_elasticity_tensor", true, "Parameter to save the old elasticity tensor values, set to true when the elasticity tensor components change, i.e. with temperature");
  return params;
}

ComputeVariableIsotropicElasticityTensor::ComputeVariableIsotropicElasticityTensor(const InputParameters & parameters) :
    ComputeIsotropicElasticityTensor(parameters),
    _temperature(coupledValue("temperature")),
    _elasticity_tensor_old(declarePropertyOld<RankFourTensor>(_elasticity_tensor_name))
{
  if (_lambda_set && _shear_modulus_set)
  {
    _youngs_modulus = _shear_modulus * (3.0 * _lambda + 2.0 * _shear_modulus) / (_lambda + _shear_modulus);
    _poissons_ratio = _lambda / (2.0 * (_lambda + _shear_modulus));
  }
  else if (_shear_modulus_set && _bulk_modulus_set)
  {
    _youngs_modulus = 9.0 * _bulk_modulus * _shear_modulus / (3.0 * _bulk_modulus + _shear_modulus);
    _poissons_ratio = (3.0 * _bulk_modulus - 2.0 * _shear_modulus) / (2.0 * (3.0 * _bulk_modulus + _shear_modulus));
  }
}

void
ComputeVariableIsotropicElasticityTensor::initQpStatefulProperties()
{
}

void
ComputeVariableIsotropicElasticityTensor::computeQpElasticityTensor()
{
  Real youngs_modulus = _youngs_modulus;
  //Create a nonphysical change in the young's modulus with temperature
  if (MooseUtils::relativeFuzzyGreaterEqual(_temperature[_qp], 600.0))
  {
    youngs_modulus -= 100.0 * _temperature[_qp];
    if (MooseUtils::relativeFuzzyLessThan(youngs_modulus, 1.0e3))
      youngs_modulus = 1.0e3;
  }

  // Build the vector to pass into RankFourTensor to create the elasticity tensor
  std::vector<Real> isotropic_elastic_constants(2);

  // lambda
  isotropic_elastic_constants[0] = youngs_modulus * _poissons_ratio / ((1.0 + _poissons_ratio) * (1.0 - 2.0 * _poissons_ratio));

  // shear modulus
  isotropic_elastic_constants[1] = youngs_modulus / (2.0 * (1.0 + _poissons_ratio));
  _Cijkl.fillFromInputVector(isotropic_elastic_constants, RankFourTensor::symmetric_isotropic);

  //Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;
}
