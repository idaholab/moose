//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMultiplePowerLawCreepStressUpdate.h"
#include <algorithm>

registerMooseObject("TensorMechanicsApp", ADMultiplePowerLawCreepStressUpdate);

InputParameters
ADMultiplePowerLawCreepStressUpdate::validParams()
{
  InputParameters params = ADRadialReturnCreepStressUpdateBase::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a radial return isotropic power law creep "
      "model. This class can be used in conjunction with other creep and plasticity materials "
      "for more complex simulations.");

  // Linear strain hardening parameters
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<std::vector<Real>>("coefficient",
                                             "Leading coefficient in power-law equation");
  params.addRequiredParam<std::vector<Real>>("n_exponent",
                                             "Exponent on effective stress in power-law equation");
  params.addRequiredParam<std::vector<Real>>("m_exponent",
                                             "Exponent on time in power-law equation");
  params.addRequiredParam<std::vector<Real>>("stress_thresholds",
                                             "Stress intervals to switch creep behavior");
  params.addRequiredParam<std::vector<Real>>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0.0, "Start time (if not zero)");
  return params;
}

ADMultiplePowerLawCreepStressUpdate::ADMultiplePowerLawCreepStressUpdate(
    const InputParameters & parameters)
  : ADRadialReturnCreepStressUpdateBase(parameters),
    _temperature(isParamValid("temperature") ? &adCoupledValue("temperature") : nullptr),
    _coefficient(getParam<std::vector<Real>>("coefficient")),
    _n_exponent(getParam<std::vector<Real>>("n_exponent")),
    _m_exponent(getParam<std::vector<Real>>("m_exponent")),
    _stress_thresholds(getParam<std::vector<Real>>("stress_thresholds")),
    _activation_energy(getParam<std::vector<Real>>("activation_energy")),
    _gas_constant(getParam<Real>("gas_constant")),
    _start_time(getParam<Real>("start_time")),
    _exponential(1),
    _exp_time(0),
    _stress_index(0),
    _number_of_models(_m_exponent.size())
{

  if (!std::is_sorted(_stress_thresholds.begin(), _stress_thresholds.end()))
    paramError("stress_thresholds",
               "Stress thresholds input must be provided in increasing ordered");

  if (_coefficient.size() != _n_exponent.size() || _coefficient.size() != _m_exponent.size() ||
      _coefficient.size() != _activation_energy.size())
    paramError(
        "n_exponent",
        "Inputs to ADMultiplePowerLawCreepStressUpdate creep models must have the same size");

  if (_number_of_models != _stress_thresholds.size() - 1)
    paramError("stress_thresholds",
               "The number of creep models must be the number of stress thresholds minute. That "
               "is, one creep model will be valid between two stress thresholds");

  for (unsigned int i = 0; i < _number_of_models; i++)
    if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent[i]) != _m_exponent[i]))
      paramError("start_time",
                 "Start time must be equal to or greater than the Executioner start_time if a "
                 "non-integer m_exponent is used");
}

void
ADMultiplePowerLawCreepStressUpdate::computeStressInitialize(
    const ADReal & effective_trial_stress, const ADRankFourTensor & /*elasticity_tensor*/)
{
  _stress_index = stressIndex(effective_trial_stress);

  if (_temperature)
    _exponential =
        std::exp(-_activation_energy[_stress_index] / (_gas_constant * (*_temperature)[_qp]));

  _exp_time = std::pow(_t - _start_time, _m_exponent[_stress_index]);
}

ADReal
ADMultiplePowerLawCreepStressUpdate::computeResidual(const ADReal & effective_trial_stress,
                                                     const ADReal & scalar)
{
  const ADReal stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ADReal creep_rate = _coefficient[_stress_index] *
                            std::pow(stress_delta, _n_exponent[_stress_index]) * _exponential *
                            _exp_time;
  return creep_rate * _dt - scalar;
}

ADReal
ADMultiplePowerLawCreepStressUpdate::computeDerivative(const ADReal & effective_trial_stress,
                                                       const ADReal & scalar)
{
  const ADReal stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ADReal creep_rate_derivative =
      -_coefficient[_stress_index] * _three_shear_modulus * _n_exponent[_stress_index] *
      std::pow(stress_delta, _n_exponent[_stress_index] - 1.0) * _exponential * _exp_time;
  return creep_rate_derivative * _dt - 1.0;
}

Real
ADMultiplePowerLawCreepStressUpdate::computeStrainEnergyRateDensity(
    const ADMaterialProperty<RankTwoTensor> & stress,
    const ADMaterialProperty<RankTwoTensor> & strain_rate)
{
  if (_n_exponent[_stress_index] <= 1)
    return 0.0;

  Real creep_factor = _n_exponent[_stress_index] / (_n_exponent[_stress_index] + 1);

  return MetaPhysicL::raw_value(creep_factor * stress[_qp].doubleContraction((strain_rate)[_qp]));
}

bool
ADMultiplePowerLawCreepStressUpdate::substeppingCapabilityEnabled()
{
  return this->_use_substepping != ADRadialReturnStressUpdate::SubsteppingType::NONE;
}

std::size_t
ADMultiplePowerLawCreepStressUpdate::stressIndex(const ADReal & effective_trial_stress)
{
  // Check the correct model for *this* radial return
  std::size_t i = 0;
  while (i < _number_of_models - 1 && effective_trial_stress > _stress_thresholds[i + 1])
    ++i;

  return i;
}
