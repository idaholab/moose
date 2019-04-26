//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPowerLawCreepStressUpdate.h"

registerADMooseObject("TensorMechanicsApp", ADPowerLawCreepStressUpdate);

defineADValidParams(
    ADPowerLawCreepStressUpdate,
    ADRadialReturnCreepStressUpdateBase,
    params.addClassDescription(
        "This class uses the stress update material in a radial return isotropic power law creep "
        "model. This class can be used in conjunction with other creep and plasticity materials "
        "for more complex simulations.");

    // Linear strain hardening parameters
    params.addCoupledVar("temperature", "Coupled temperature");
    params.addRequiredParam<Real>("coefficient", "Leading coefficient in power-law equation");
    params.addRequiredParam<Real>("n_exponent",
                                  "Exponent on effective stress in power-law equation");
    params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
    params.addRequiredParam<Real>("activation_energy", "Activation energy");
    params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
    params.addParam<Real>("start_time", 0.0, "Start time (if not zero)"););

template <ComputeStage compute_stage>
ADPowerLawCreepStressUpdate<compute_stage>::ADPowerLawCreepStressUpdate(
    const InputParameters & parameters)
  : ADRadialReturnCreepStressUpdateBase<compute_stage>(parameters),
    _has_temp(isParamValid("temperature")),
    _coefficient(adGetParam<Real>("coefficient")),
    _n_exponent(adGetParam<Real>("n_exponent")),
    _m_exponent(adGetParam<Real>("m_exponent")),
    _activation_energy(adGetParam<Real>("activation_energy")),
    _gas_constant(adGetParam<Real>("gas_constant")),
    _start_time(adGetParam<Real>("start_time"))
{
  if (_has_temp)
    _temperature = &adCoupledValue("temperature");

  if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    paramError("start_time",
               "Start time must be equal to or greater than the Executioner start_time if a "
               "non-integer m_exponent is used");
}

template <ComputeStage compute_stage>
void
ADPowerLawCreepStressUpdate<compute_stage>::computeStressInitialize(
    const ADReal & /*effective_trial_stress*/, const ADRankFourTensor & /*elasticity_tensor*/)
{
  if (_has_temp)
    _exponential = std::exp(-_activation_energy / (_gas_constant * (*_temperature)[_qp]));
  else
    _exponential = 1.0;

  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

template <ComputeStage compute_stage>
ADReal
ADPowerLawCreepStressUpdate<compute_stage>::computeResidual(const ADReal & effective_trial_stress,
                                                            const ADReal & scalar)
{
  const ADReal stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ADReal creep_rate =
      _coefficient * std::pow(stress_delta, _n_exponent) * _exponential * _exp_time;
  return creep_rate * _dt - scalar;
}

template <ComputeStage compute_stage>
ADReal
ADPowerLawCreepStressUpdate<compute_stage>::computeDerivative(const ADReal & effective_trial_stress,
                                                              const ADReal & scalar)
{
  const ADReal stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ADReal creep_rate_derivative = -_coefficient * _three_shear_modulus * _n_exponent *
                                       std::pow(stress_delta, _n_exponent - 1.0) * _exponential *
                                       _exp_time;
  return creep_rate_derivative * _dt - 1.0;
}
