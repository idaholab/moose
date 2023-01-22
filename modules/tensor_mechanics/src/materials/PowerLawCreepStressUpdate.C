//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PowerLawCreepStressUpdate.h"

registerMooseObject("TensorMechanicsApp", PowerLawCreepStressUpdate);
registerMooseObject("TensorMechanicsApp", ADPowerLawCreepStressUpdate);

template <bool is_ad>
InputParameters
PowerLawCreepStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams();
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

template <bool is_ad>
PowerLawCreepStressUpdateTempl<is_ad>::PowerLawCreepStressUpdateTempl(
    const InputParameters & parameters)
  : RadialReturnCreepStressUpdateBaseTempl<is_ad>(parameters),
    _temperature(this->isParamValid("temperature")
                     ? &this->template coupledGenericValue<is_ad>("temperature")
                     : nullptr),
    _coefficient(this->template getParam<Real>("coefficient")),
    _n_exponent(this->template getParam<Real>("n_exponent")),
    _m_exponent(this->template getParam<Real>("m_exponent")),
    _activation_energy(this->template getParam<Real>("activation_energy")),
    _gas_constant(this->template getParam<Real>("gas_constant")),
    _start_time(this->template getParam<Real>("start_time")),
    _exponential(1.0)
{
  if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    this->paramError("start_time",
                     "Start time must be equal to or greater than the Executioner start_time if a "
                     "non-integer m_exponent is used");
}

template <bool is_ad>
void
PowerLawCreepStressUpdateTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & /*effective_trial_stress*/,
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
{
  if (_temperature)
    _exponential = std::exp(-_activation_energy / (_gas_constant * (*_temperature)[_qp]));

  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

template <bool is_ad>
template <typename ScalarType>
ScalarType
PowerLawCreepStressUpdateTempl<is_ad>::computeResidualInternal(
    const GenericReal<is_ad> & effective_trial_stress, const ScalarType & scalar)
{
  const ScalarType stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const ScalarType creep_rate =
      _coefficient * std::pow(stress_delta, _n_exponent) * _exponential * _exp_time;
  return creep_rate * _dt - scalar;
}

template <bool is_ad>
GenericReal<is_ad>
PowerLawCreepStressUpdateTempl<is_ad>::computeDerivative(
    const GenericReal<is_ad> & effective_trial_stress, const GenericReal<is_ad> & scalar)
{
  const GenericReal<is_ad> stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  const GenericReal<is_ad> creep_rate_derivative =
      -_coefficient * _three_shear_modulus * _n_exponent *
      std::pow(stress_delta, _n_exponent - 1.0) * _exponential * _exp_time;
  return creep_rate_derivative * _dt - 1.0;
}

template <bool is_ad>
Real
PowerLawCreepStressUpdateTempl<is_ad>::computeStrainEnergyRateDensity(
    const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
    const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate)
{
  if (_n_exponent <= 1)
    return 0.0;

  Real creep_factor = _n_exponent / (_n_exponent + 1);

  return MetaPhysicL::raw_value(creep_factor * stress[_qp].doubleContraction((strain_rate)[_qp]));
}

template <bool is_ad>
void
PowerLawCreepStressUpdateTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _creep_strain[_qp] += plastic_strain_increment;
}

template <bool is_ad>
void
PowerLawCreepStressUpdateTempl<is_ad>::resetIncrementalMaterialProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

template <bool is_ad>
bool
PowerLawCreepStressUpdateTempl<is_ad>::substeppingCapabilityEnabled()
{
  return this->_use_substepping != RadialReturnStressUpdateTempl<is_ad>::SubsteppingType::NONE;
}

template class PowerLawCreepStressUpdateTempl<false>;
template class PowerLawCreepStressUpdateTempl<true>;
template Real PowerLawCreepStressUpdateTempl<false>::computeResidualInternal<Real>(const Real &,
                                                                                   const Real &);
template ADReal
PowerLawCreepStressUpdateTempl<true>::computeResidualInternal<ADReal>(const ADReal &,
                                                                      const ADReal &);
template ChainedReal
PowerLawCreepStressUpdateTempl<false>::computeResidualInternal<ChainedReal>(const Real &,
                                                                            const ChainedReal &);
template ChainedADReal
PowerLawCreepStressUpdateTempl<true>::computeResidualInternal<ChainedADReal>(const ADReal &,
                                                                             const ChainedADReal &);
