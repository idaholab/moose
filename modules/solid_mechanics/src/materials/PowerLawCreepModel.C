/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PowerLawCreepModel.h"

#include "SymmIsotropicElasticityTensor.h"

template <>
InputParameters
validParams<PowerLawCreepModel>()
{
  InputParameters params = validParams<ReturnMappingModel>();

  // Power-law creep material parameters
  params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0, "Start time (if not zero)");
  params.addCoupledVar("temp", "Coupled Temperature");

  return params;
}

PowerLawCreepModel::PowerLawCreepModel(const InputParameters & parameters)
  : ReturnMappingModel(parameters),
    _coefficient(parameters.get<Real>("coefficient")),
    _n_exponent(parameters.get<Real>("n_exponent")),
    _m_exponent(parameters.get<Real>("m_exponent")),
    _activation_energy(parameters.get<Real>("activation_energy")),
    _gas_constant(parameters.get<Real>("gas_constant")),
    _start_time(getParam<Real>("start_time")),
    _creep_strain(declareProperty<SymmTensor>("creep_strain")),
    _creep_strain_old(declarePropertyOld<SymmTensor>("creep_strain"))
{
}

void
PowerLawCreepModel::computeStressInitialize(unsigned qp,
                                            Real /*effectiveTrialStress*/,
                                            const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
  {
    mooseError("PowerLawCreepModel requires a SymmIsotropicElasticityTensor");
  }
  _shear_modulus = eT->shearModulus();

  _exponential = 1;
  if (_has_temp)
  {
    _exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[qp]));
  }

  _expTime = std::pow(_t - _start_time, _m_exponent);

  _creep_strain[qp] = _creep_strain_old[qp];
}

void
PowerLawCreepModel::computeStressFinalize(unsigned qp, const SymmTensor & plasticStrainIncrement)
{
  _creep_strain[qp] += plasticStrainIncrement;
}

Real
PowerLawCreepModel::computeResidual(unsigned /*qp*/, Real effectiveTrialStress, Real scalar)
{
  return _coefficient * std::pow(effectiveTrialStress - 3 * _shear_modulus * scalar, _n_exponent) *
             _exponential * _expTime -
         scalar / _dt;
}

Real
PowerLawCreepModel::computeDerivative(unsigned /*qp*/, Real effectiveTrialStress, Real scalar)
{
  return -3 * _coefficient * _shear_modulus * _n_exponent *
             std::pow(effectiveTrialStress - 3 * _shear_modulus * scalar, _n_exponent - 1) *
             _exponential * _expTime -
         1 / _dt;
}
