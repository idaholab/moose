//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemperatureDependentHardeningStressUpdate.h"

#include "PiecewiseLinear.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", TemperatureDependentHardeningStressUpdate);
registerMooseObject("TensorMechanicsApp", ADTemperatureDependentHardeningStressUpdate);

template <bool is_ad>
InputParameters
TemperatureDependentHardeningStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = IsotropicPlasticityStressUpdateTempl<is_ad>::validParams();
  params.addClassDescription("Computes the stress as a function of temperature "
                             "and plastic strain from user-supplied hardening "
                             "functions. This class can be used in conjunction "
                             "with other creep and plasticity materials for "
                             "more complex simulations");
  params.set<Real>("yield_stress") = 1.0;
  params.set<Real>("hardening_constant") = 1.0;

  params.suppressParameter<Real>("yield_stress");
  params.suppressParameter<FunctionName>("yield_stress_function");
  params.suppressParameter<Real>("hardening_constant");
  params.suppressParameter<FunctionName>("hardening_function");

  params.addRequiredParam<std::vector<FunctionName>>(
      "hardening_functions",
      "List of functions of true stress as function of plastic strain at different temperatures");
  params.addRequiredParam<std::vector<Real>>(
      "temperatures",
      "List of temperatures corresponding to the functions listed in 'hardening_functions'");

  return params;
}

template <bool is_ad>
TemperatureDependentHardeningStressUpdateTempl<
    is_ad>::TemperatureDependentHardeningStressUpdateTempl(const InputParameters & parameters)
  : IsotropicPlasticityStressUpdateTempl<is_ad>(parameters),
    _hardening_functions_names(
        this->template getParam<std::vector<FunctionName>>("hardening_functions")),
    _hf_temperatures(this->template getParam<std::vector<Real>>("temperatures"))
{
  const unsigned int len = _hardening_functions_names.size();
  if (len < 2)
    mooseError("At least two stress-strain curves must be provided in hardening_functions");
  _hardening_functions.resize(len);

  const unsigned int len_temps = _hf_temperatures.size();
  if (len != len_temps)
    mooseError("The vector of hardening function temperatures must have the same length as the "
               "vector of temperature dependent hardening functions.");

  // Check that the temperatures are strictly increasing
  for (unsigned int i = 1; i < len_temps; ++i)
    if (_hf_temperatures[i] <= _hf_temperatures[i - 1])
      mooseError("The temperature dependent hardening functions and corresponding temperatures "
                 "should be listed in order of increasing temperature.");

  std::vector<Real> yield_stress_vec;
  for (unsigned int i = 0; i < len; ++i)
  {
    const PiecewiseLinear * const f = dynamic_cast<const PiecewiseLinear *>(
        &this->getFunctionByName(_hardening_functions_names[i]));
    if (!f)
      mooseError("Function ", _hardening_functions_names[i], " not found in ", this->name());

    _hardening_functions[i] = f;

    yield_stress_vec.push_back(f->value(0.0, Point()));
  }

  _interp_yield_stress = std::make_unique<LinearInterpolation>(_hf_temperatures, yield_stress_vec);
}

template <bool is_ad>
void
TemperatureDependentHardeningStressUpdateTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & effectiveTrialStress,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  initializeHardeningFunctions();
  computeYieldStress(elasticity_tensor);

  this->_yield_condition =
      effectiveTrialStress - this->_hardening_variable_old[_qp] - this->_yield_stress;
  this->_hardening_variable[_qp] = this->_hardening_variable_old[_qp];
  this->_plastic_strain[_qp] = this->_plastic_strain_old[_qp];
}

template <bool is_ad>
void
TemperatureDependentHardeningStressUpdateTempl<is_ad>::initializeHardeningFunctions()
{
  const Real temp = MetaPhysicL::raw_value(this->_temperature[_qp]);
  if (temp > _hf_temperatures[0] && temp < _hf_temperatures.back())
  {
    for (unsigned int i = 0; i < _hf_temperatures.size() - 1; ++i)
    {
      if (temp >= _hf_temperatures[i] && temp < _hf_temperatures[i + 1])
      {
        _hf_index_lo = i;
        _hf_index_hi = i + 1;
        _hf_fraction =
            (temp - _hf_temperatures[i]) / (_hf_temperatures[i + 1] - _hf_temperatures[i]);
      }
    }
  }
  else if (temp <= _hf_temperatures[0])
  {
    _hf_index_lo = 0;
    _hf_index_hi = _hf_index_lo;
    _hf_fraction = 0.0;
  }
  else if (temp >= _hf_temperatures.back())
  {
    _hf_index_lo = _hf_temperatures.size() - 1;
    _hf_index_hi = _hf_index_lo;
    _hf_fraction = 1.0;
  }

  if (_hf_fraction < 0.0)
    mooseError("The hardening function fraction cannot be less than zero.");
}

template <bool is_ad>
GenericReal<is_ad>
TemperatureDependentHardeningStressUpdateTempl<is_ad>::computeHardeningValue(
    const GenericReal<is_ad> & scalar)
{
  const Real strain = this->_effective_inelastic_strain_old[_qp] + MetaPhysicL::raw_value(scalar);
  const GenericReal<is_ad> stress =
      (1.0 - _hf_fraction) * _hardening_functions[_hf_index_lo]->value(strain, Point()) +
      _hf_fraction * _hardening_functions[_hf_index_hi]->value(strain, Point());

  return stress - this->_yield_stress;
}

template <bool is_ad>
GenericReal<is_ad>
TemperatureDependentHardeningStressUpdateTempl<is_ad>::computeHardeningDerivative(
    const GenericReal<is_ad> & /*scalar*/)
{
  const Real strain_old = this->_effective_inelastic_strain_old[_qp];

  return (1.0 - _hf_fraction) *
             _hardening_functions[_hf_index_lo]->timeDerivative(strain_old, Point()) +
         _hf_fraction * _hardening_functions[_hf_index_hi]->timeDerivative(strain_old, Point());
}

template <bool is_ad>
void
TemperatureDependentHardeningStressUpdateTempl<is_ad>::computeYieldStress(
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
{
  this->_yield_stress =
      _interp_yield_stress->sample(MetaPhysicL::raw_value(this->_temperature[_qp]));
  if (this->_yield_stress <= 0.0)
    mooseError("The yield stress must be greater than zero, but during the simulation your yield "
               "stress became less than zero.");
}

template class TemperatureDependentHardeningStressUpdateTempl<false>;
template class TemperatureDependentHardeningStressUpdateTempl<true>;
