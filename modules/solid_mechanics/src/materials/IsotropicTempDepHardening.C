/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "IsotropicTempDepHardening.h"

#include "SymmIsotropicElasticityTensor.h"

#include "PiecewiseLinear.h"

template <>
InputParameters
validParams<IsotropicTempDepHardening>()
{
  InputParameters params = validParams<IsotropicPlasticity>();

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

IsotropicTempDepHardening::IsotropicTempDepHardening(const InputParameters & parameters)
  : IsotropicPlasticity(parameters),
    _hardening_functions_names(getParam<std::vector<FunctionName>>("hardening_functions")),
    _hf_temperatures(getParam<std::vector<Real>>("temperatures"))
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
  {
    if (_hf_temperatures[i] <= _hf_temperatures[i - 1])
      mooseError("The temperature dependent hardening functions and corresponding temperatures "
                 "should be listed in order of increasing temperature.");
  }

  std::vector<Real> yield_stress_vec;
  for (unsigned int i = 0; i < len; ++i)
  {
    PiecewiseLinear * const f =
        dynamic_cast<PiecewiseLinear *>(&getFunctionByName(_hardening_functions_names[i]));
    if (!f)
      mooseError("Function ", _hardening_functions_names[i], " not found in ", name());

    _hardening_functions[i] = f;

    yield_stress_vec.push_back(f->value(0.0, Point()));
  }

  _interp_yield_stress = MooseSharedPointer<LinearInterpolation>(
      new LinearInterpolation(_hf_temperatures, yield_stress_vec));

  _scalar_plastic_strain = &declareProperty<Real>("scalar_plastic_strain");
  _scalar_plastic_strain_old = &declarePropertyOld<Real>("scalar_plastic_strain");
}

void
IsotropicTempDepHardening::computeStressInitialize(unsigned qp,
                                                   Real effectiveTrialStress,
                                                   const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
    mooseError("IsotropicPlasticity requires a SymmIsotropicElasticityTensor");

  _shear_modulus = eT->shearModulus();
  initializeHardeningFunctions(qp);
  computeYieldStress(qp);
  _yield_condition = effectiveTrialStress - _hardening_variable_old[qp] - _yield_stress;
  _hardening_variable[qp] = _hardening_variable_old[qp];
  _plastic_strain[qp] = _plastic_strain_old[qp];
}

void
IsotropicTempDepHardening::initializeHardeningFunctions(unsigned qp)
{
  const Real temp = _temperature[qp];
  if (temp > _hf_temperatures[0] && temp < _hf_temperatures.back())
  {
    for (unsigned int i = 0; i < _hf_temperatures.size() - 1; ++i)
    {
      if (temp >= _hf_temperatures[i] && temp < _hf_temperatures[i + 1])
      {
        _hf_index_lo = i;
        _hf_index_hi = i + 1;
        Real temp_lo = _hf_temperatures[i];
        Real temp_hi = _hf_temperatures[i + 1];
        _hf_fraction = (temp - temp_lo) / (temp_hi - temp_lo);
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

Real
IsotropicTempDepHardening::computeHardeningValue(unsigned qp, Real scalar)
{
  const Real strain = (*_scalar_plastic_strain_old)[qp] + scalar;

  const Real stress =
      (1.0 - _hf_fraction) * _hardening_functions[_hf_index_lo]->value(strain, Point()) +
      _hf_fraction * _hardening_functions[_hf_index_hi]->value(strain, Point());

  return stress - _yield_stress;
}

Real
IsotropicTempDepHardening::computeHardeningDerivative(unsigned qp, Real /*scalar*/)
{
  const Real strain_old = (*_scalar_plastic_strain_old)[qp];

  const Real derivative =
      (1.0 - _hf_fraction) *
          _hardening_functions[_hf_index_lo]->timeDerivative(strain_old, Point()) +
      _hf_fraction * _hardening_functions[_hf_index_hi]->timeDerivative(strain_old, Point());

  return derivative;
}

void
IsotropicTempDepHardening::computeYieldStress(unsigned qp)
{
  _yield_stress = _interp_yield_stress->sample(_temperature[qp]);
  if (_yield_stress <= 0.0)
    mooseError("The yield stress must be greater than zero, but during the simulation your yield "
               "stress became less than zero.");
}
