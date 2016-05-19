/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "IsotropicTempDepHardening.h"

#include "SymmIsotropicElasticityTensor.h"

#include "PiecewiseLinear.h"

template<>
InputParameters validParams<IsotropicTempDepHardening>()
{
  InputParameters params = validParams<IsotropicPlasticity>();

  params.set<Real>("yield_stress") = 1.0;
  params.set<Real>("hardening_constant") = 1.0;

  params.suppressParameter<Real>("yield_stress");
  params.suppressParameter<FunctionName>("yield_stress_function");
  params.suppressParameter<Real>("hardening_constant");
  params.suppressParameter<FunctionName>("hardening_function");

  params.addRequiredParam<std::vector<FunctionName> >("hardening_functions", "List of functions of true stress as function of plastic strain at different temperatures");
  params.addRequiredParam<std::vector<Real> >("temperatures", "List of temperatures for the true stress as a function of plastic strain curves");

  return params;
}

IsotropicTempDepHardening::IsotropicTempDepHardening(const InputParameters & parameters) :
    IsotropicPlasticity(parameters),
    _hardening_functions_names(getParam<std::vector<FunctionName> >("hardening_functions")),
    _hf_temperatures(getParam<std::vector<Real> >("temperatures"))
{
  const unsigned len = _hardening_functions_names.size();
  if (len < 2)
    mooseError("At least two stress-strain curves must be provided in hardening_functions");
  _hardening_functions.resize(len);

  const unsigned len_temps = _hf_temperatures.size();
  if (len != len_temps)
    mooseError("The vector of hardening function temperatures must have the same length as the vector of temperature dependent hardening functions.");

  //Check that the temperatures are strictly increasing
  for (unsigned int i = 1; i < len_temps; ++i)
  {
    if (_hf_temperatures[i] <= _hf_temperatures[i-1])
      mooseError("The temperature dependent hardening functions and corresponding temperatures should be listed in order of increasing temperature.");
  }

  std::vector<Real> yield_stress_vec;
  for (unsigned int i = 0; i < len; ++i)
  {
    PiecewiseLinear * const f = dynamic_cast<PiecewiseLinear*>(&getFunctionByName(_hardening_functions_names[i]));
    if (!f)
      mooseError("Function " << _hardening_functions_names[i] << " not found in " << name());

    _hardening_functions[i] = f;

    Point p;
    yield_stress_vec.push_back(f->value(0.0, p));
  }

  _interp_yield_stress = new LinearInterpolation(_hf_temperatures, yield_stress_vec);

  // _scalar_plastic_strain = &declareProperty<Real>("scalar_plastic_strain");
  // _scalar_plastic_strain_old = &declarePropertyOld<Real>("scalar_plastic_strain");
}

Real
IsotropicTempDepHardening::computeHardeningDerivative(unsigned qp, Real /*scalar*/)
{
  const Real strain_old = (*_scalar_plastic_strain_old)[qp];
  Real temp = _temperature[qp];
  Real derivative = 0.0;
  Point p;

  if (temp > _hf_temperatures[0] && temp < _hf_temperatures.back())
  {
    //Find between which curves this temperature falls
    for (unsigned int i = 0; i < _hf_temperatures.size() - 1; ++i)
    {
      if (temp > _hf_temperatures[i] && temp < _hf_temperatures[i+1])
      {
        Real temp_lo = _hf_temperatures[i];
        Real temp_hi = _hf_temperatures[i+1];
        Real derivative_temp_lo = _hardening_functions[i]->timeDerivative(strain_old, p);
        Real derivative_temp_hi = _hardening_functions[i+1]->timeDerivative(strain_old, p);
        derivative = derivative_temp_lo + (derivative_temp_hi - derivative_temp_lo) * ((temp - temp_lo) / (temp_hi - temp_lo));
      }
    }
  }

  else if (temp < _hf_temperatures[0])
    derivative = _hardening_functions[0]->timeDerivative(strain_old, p);

  else if (temp > _hf_temperatures.back())
    derivative = _hardening_functions.back()->timeDerivative(strain_old, p);

  return derivative;
}

Real
IsotropicTempDepHardening::computeHardeningValue(unsigned qp, Real scalar)
{
  const Real strain = (*_scalar_plastic_strain_old)[qp] + scalar;
  Real temp = _temperature[qp];
  Real stress = 0.0;
  Point p;

  if (temp > _hf_temperatures[0] && temp < _hf_temperatures.back())
  {
    for (unsigned int i = 0; i < _hf_temperatures.size() - 1; ++i)
    {
      if (temp > _hf_temperatures[i] && temp < _hf_temperatures[i+1])
      {
        Real temp_lo = _hf_temperatures[i];
        Real temp_hi = _hf_temperatures[i+1];
        Real stress_temp_lo = _hardening_functions[i]->value(strain, p);
        Real stress_temp_hi = _hardening_functions[i+1]->value(strain, p);
        stress = stress_temp_lo + (stress_temp_hi - stress_temp_lo) * ((temp - temp_lo) / (temp_hi - temp_lo));
      }
    }
  }

  else if (temp < _hf_temperatures[0])
    stress = _hardening_functions[0]->value(strain, p);

  else if (temp > _hf_temperatures.back())
    stress = _hardening_functions.back()->value(strain, p);

  return stress - _yield_stress;
}

void
IsotropicTempDepHardening::computeYieldStress(unsigned qp)
{
  _yield_stress = _interp_yield_stress->sample(_temperature[qp]);

  if (_yield_stress <= 0)
    mooseError("Yield stress must be greater than zero");
}
