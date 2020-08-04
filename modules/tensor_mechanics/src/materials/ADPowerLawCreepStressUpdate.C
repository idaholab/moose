//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPowerLawCreepStressUpdate.h"

registerMooseObject("TensorMechanicsApp", ADPowerLawCreepStressUpdate);

InputParameters
ADPowerLawCreepStressUpdate::validParams()
{
  InputParameters params = ADRadialReturnCreepStressUpdateBase::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a radial return isotropic power law creep "
      "model. This class can be used in conjunction with other creep and plasticity materials "
      "for more complex simulations.");

  // Linear strain hardening parameters
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<Real>("coefficient", "Leading coefficient in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0.0, "Start time (if not zero)");
  return params;
}

ADPowerLawCreepStressUpdate::ADPowerLawCreepStressUpdate(const InputParameters & parameters)
  : ADRadialReturnCreepStressUpdateBase(parameters),
    _temperature(isParamValid("temperature") ? &adCoupledValue("temperature") : nullptr),
    _coefficient(getParam<Real>("coefficient")),
    _n_exponent(getParam<Real>("n_exponent")),
    _m_exponent(getParam<Real>("m_exponent")),
    _activation_energy(getParam<Real>("activation_energy")),
    _gas_constant(getParam<Real>("gas_constant")),
    _start_time(getParam<Real>("start_time")),
    _exponential(1.0)
{
  if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    paramError("start_time",
               "Start time must be equal to or greater than the Executioner start_time if a "
               "non-integer m_exponent is used");
}

void
ADPowerLawCreepStressUpdate::computeStressInitialize(const ADReal & /*effective_trial_stress*/,
                                                     const ADRankFourTensor & /*elasticity_tensor*/)
{
  if (_temperature)
    _exponential = std::exp(-_activation_energy / (_gas_constant * (*_temperature)[_qp]));

  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

ADReal
ADPowerLawCreepStressUpdate::computeResidual(const ADReal & effective_trial_stress,
                                             const ADReal & scalar)
{
  const ADReal stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ADReal creep_rate =
      _coefficient * std::pow(stress_delta, _n_exponent) * _exponential * _exp_time;
  return creep_rate * _dt - scalar;
}

ADReal
ADPowerLawCreepStressUpdate::computeDerivative(const ADReal & effective_trial_stress,
                                               const ADReal & scalar)
{
  const ADReal stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ADReal creep_rate_derivative = -_coefficient * _three_shear_modulus * _n_exponent *
                                       std::pow(stress_delta, _n_exponent - 1.0) * _exponential *
                                       _exp_time;
  return creep_rate_derivative * _dt - 1.0;
}

ADReal
ADPowerLawCreepStressUpdate::computeStrainEnergyRateDensity(
    const ADMaterialProperty<RankTwoTensor> & stress,
    const ADMaterialProperty<RankTwoTensor> & strain_rate)
{
  if (_n_exponent <= 1)
    return 0.0;

  Real creep_factor = _n_exponent / (_n_exponent + 1);

  return creep_factor * stress[_qp].doubleContraction((strain_rate)[_qp]);
}
