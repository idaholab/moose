//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositePowerLawCreepStressUpdate.h"
#include <cmath>

registerMooseObject("SolidMechanicsApp", CompositePowerLawCreepStressUpdate);
registerMooseObject("SolidMechanicsApp", ADCompositePowerLawCreepStressUpdate);

template <bool is_ad>
InputParameters
CompositePowerLawCreepStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a radial return isotropic power law creep "
      "model. This class can be used in conjunction with other creep and plasticity materials "
      "for more complex simulations. This class is an extension to include multi-phase "
      "capability.");

  // Linear strain hardening parameters
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<std::vector<Real>>(
      "coefficient",
      "a vector of leading coefficient / Dorn Constant in power-law equation for each material.");
  params.addRequiredParam<std::vector<Real>>(
      "n_exponent",
      "a vector of Exponent on effective stress in power-law equation for each material");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<std::vector<Real>>("activation_energy",
                                             "a vector of Activation energy for Arrhenius-type "
                                             "equation of the Dorn Constant for each material");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0.0, "Start time (if not zero)");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "switching_functions", "a vector of switching functions for each material");
  return params;
}

template <bool is_ad>
CompositePowerLawCreepStressUpdateTempl<is_ad>::CompositePowerLawCreepStressUpdateTempl(
    const InputParameters & parameters)
  : RadialReturnCreepStressUpdateBaseTempl<is_ad>(parameters),
    _temperature(this->isParamValid("temperature")
                     ? &this->template coupledGenericValue<is_ad>("temperature")
                     : nullptr),
    _coefficient(this->template getParam<std::vector<Real>>("coefficient")),
    _n_exponent(this->template getParam<std::vector<Real>>("n_exponent")),
    _m_exponent(this->template getParam<Real>("m_exponent")),
    _activation_energy(this->template getParam<std::vector<Real>>("activation_energy")),
    _gas_constant(this->template getParam<Real>("gas_constant")),
    _start_time(this->template getParam<Real>("start_time")),
    _switching_func_names(
        this->template getParam<std::vector<MaterialPropertyName>>("switching_functions"))

{
  _num_materials = _switching_func_names.size();
  if (_n_exponent.size() != _num_materials)
    this->paramError("n_exponent", "n exponent must be equal to the number of switching functions");

  if (_coefficient.size() != _num_materials)
    this->paramError("coefficient",
                     "number of Dorn constant must be equal to the number of switching functions");

  if (_activation_energy.size() != _num_materials)
    this->paramError("activation_energy",
                     "activation energy must be equal to the number of swithing functions");

  if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    this->paramError("start_time",
                     "Start time must be equal to or greater than the Executioner start_time if a "
                     "non-integer m_exponent is used");
  _switchingFunc.resize(_num_materials);
  // set switching functions material properties for each phase
  for (unsigned int i = 0; i < _num_materials; ++i)
  {
    _switchingFunc[i] =
        &this->template getGenericMaterialProperty<Real, is_ad>(_switching_func_names[i]);
  }
}

template <bool is_ad>
void
CompositePowerLawCreepStressUpdateTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & effective_trial_stress,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  RadialReturnStressUpdateTempl<is_ad>::computeStressInitialize(effective_trial_stress,
                                                                elasticity_tensor);
  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

template <bool is_ad>
template <typename ScalarType>
ScalarType
CompositePowerLawCreepStressUpdateTempl<is_ad>::computeResidualInternal(
    const GenericReal<is_ad> & effective_trial_stress, const ScalarType & scalar)
{
  const ScalarType stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  ScalarType creep_rate = 0.0;
  for (const auto n : make_range(_num_materials))
  {
    creep_rate +=
        _coefficient[n] * std::pow(stress_delta, _n_exponent[n]) * (*_switchingFunc[n])[_qp] *
        std::exp(-_activation_energy[n] / (_gas_constant * (*_temperature)[_qp])) * _exp_time;
  }
  return creep_rate * _dt - scalar;
}

template <bool is_ad>
GenericReal<is_ad>
CompositePowerLawCreepStressUpdateTempl<is_ad>::computeDerivative(
    const GenericReal<is_ad> & effective_trial_stress, const GenericReal<is_ad> & scalar)
{
  const GenericReal<is_ad> stress_delta = effective_trial_stress - _three_shear_modulus * scalar;
  GenericReal<is_ad> creep_rate_derivative = 0.0;
  for (const auto n : make_range(_num_materials))
  {
    creep_rate_derivative +=
        -_coefficient[n] * _three_shear_modulus * _n_exponent[n] *
        std::pow(stress_delta, _n_exponent[n] - 1.0) * (*_switchingFunc[n])[_qp] *
        std::exp(-_activation_energy[n] / (_gas_constant * (*_temperature)[_qp])) * _exp_time;
  }
  return creep_rate_derivative * _dt - 1.0;
}

template <bool is_ad>
Real
CompositePowerLawCreepStressUpdateTempl<is_ad>::computeStrainEnergyRateDensity(
    const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
    const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate)
{
  GenericReal<is_ad> interpolated_exponent = 0.0;
  for (unsigned int n = 0; n < _num_materials; ++n)
  {
    interpolated_exponent += (_n_exponent[n] / (_n_exponent[n] + 1)) * (*_switchingFunc[n])[_qp];
  }
  return MetaPhysicL::raw_value(interpolated_exponent *
                                stress[_qp].doubleContraction((strain_rate)[_qp]));
}

template <bool is_ad>
void
CompositePowerLawCreepStressUpdateTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _creep_strain[_qp] += plastic_strain_increment;
}

template <bool is_ad>
void
CompositePowerLawCreepStressUpdateTempl<is_ad>::resetIncrementalMaterialProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

template <bool is_ad>
bool
CompositePowerLawCreepStressUpdateTempl<is_ad>::substeppingCapabilityEnabled()
{
  return this->_use_substepping != RadialReturnStressUpdateTempl<is_ad>::SubsteppingType::NONE;
}

template class CompositePowerLawCreepStressUpdateTempl<false>;
template class CompositePowerLawCreepStressUpdateTempl<true>;
template Real
CompositePowerLawCreepStressUpdateTempl<false>::computeResidualInternal<Real>(const Real &,
                                                                              const Real &);
template ADReal
CompositePowerLawCreepStressUpdateTempl<true>::computeResidualInternal<ADReal>(const ADReal &,
                                                                               const ADReal &);
template ChainedReal
CompositePowerLawCreepStressUpdateTempl<false>::computeResidualInternal<ChainedReal>(
    const Real &, const ChainedReal &);
template ChainedADReal
CompositePowerLawCreepStressUpdateTempl<true>::computeResidualInternal<ChainedADReal>(
    const ADReal &, const ChainedADReal &);
