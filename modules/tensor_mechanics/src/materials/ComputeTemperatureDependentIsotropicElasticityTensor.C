/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeTemperatureDependentIsotropicElasticityTensor.h"

#include "Function.h"

template<>
InputParameters validParams<ComputeTemperatureDependentIsotropicElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an isotropic elasticity tensor for elastic constants that change as a function of temperature");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<FunctionName>("youngs_modulus_function","Function defining Young's modulus as a function of temperature");
  params.addRequiredParam<FunctionName>("poissons_ratio_function","Function defining Poisson's ratio as a function of temperature");
  return params;
}

ComputeTemperatureDependentIsotropicElasticityTensor::ComputeTemperatureDependentIsotropicElasticityTensor(const InputParameters & parameters) :
    ComputeElasticityTensorBase(parameters),
    _temperature(coupledValue("temperature")),
    _youngs_modulus_function(getFunction("youngs_modulus_function")),
    _poissons_ratio_function(getFunction("poissons_ratio_function")),
    _elasticity_tensor_old(declarePropertyOld<RankFourTensor>(_elasticity_tensor_name))
{
}

void
ComputeTemperatureDependentIsotropicElasticityTensor::initQpStatefulProperties()
{
}

void
ComputeTemperatureDependentIsotropicElasticityTensor::computeQpElasticityTensor()
{
  const Point p;
  const Real youngs_modulus = _youngs_modulus_function.value(_temperature[_qp], p);
  const Real poissons_ratio = _poissons_ratio_function.value(_temperature[_qp], p);

  // Build the vector to pass into RankFourTensor to create the elasticity tensor
  std::vector<Real> isotropic_elastic_constants(2);

  // lambda
  isotropic_elastic_constants[0] = youngs_modulus * poissons_ratio / ((1.0 + poissons_ratio) * (1.0 - 2.0 * poissons_ratio));

  // shear modulus
  isotropic_elastic_constants[1] = youngs_modulus / (2.0 * (1.0 + poissons_ratio));

  _elasticity_tensor[_qp].fillFromInputVector(isotropic_elastic_constants, RankFourTensor::symmetric_isotropic);
}
