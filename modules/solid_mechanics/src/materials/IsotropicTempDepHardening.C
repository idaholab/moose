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

  params.addRequiredParam<std::vector<FunctionName> >("temp_dep_hardening_functions", "List of functions of true stress as function of plastic strain at different temperatures");
  params.addRequiredParam<std::vector<Real> >("temp_dep_hardening_functions_temps", "List of temperatures for the true stress as a function of plastic strain curves");

  return params;
}

IsotropicTempDepHardening::IsotropicTempDepHardening(const InputParameters & parameters) :
    IsotropicPlasticity(parameters),
    _temp_dep_hardening_functions_temps(getParam<std::vector<Real> >("temp_dep_hardening_functions_temps"))
{
  const std::vector<FunctionName> & names(getParam<std::vector<FunctionName> >("temp_dep_hardening_functions"));
  const unsigned len = names.size();
  if (len < 2)
    mooseError("At least two stress-strain curves must be provided in temp_dep_hardening_functions");
  _temp_dep_hardening_functions.resize(len);

  const unsigned len_temps = _temp_dep_hardening_functions_temps.size();
  if (len != len_temps)
    mooseError("The vector of hardening function temperatures must have the same length as the vector of temperature dependent hardening functions.");

  //Check that the temperatures are strictly increasing
  for (unsigned int i = 1; i < len_temps; ++i)
  {
    if (_temp_dep_hardening_functions_temps[i] <= _temp_dep_hardening_functions_temps[i-1])
      mooseError("The temperature dependent hardening functions and corresponding temperatures should be listed in order of increasing temperature.");
  }

  std::vector<Real> yield_stress_vec;
  for (unsigned int i = 0; i < len; ++i)
  {
    PiecewiseLinear * const f = dynamic_cast<PiecewiseLinear*>(&getFunctionByName(names[i]));
    if (!f)
      mooseError("Function " << names[i] << " not found in " << name());

    _temp_dep_hardening_functions[i] = f;

    Point p;
    yield_stress_vec.push_back(f->value(0.0, p));
  }

  _interp_yield_stress = new LinearInterpolation(_temp_dep_hardening_functions_temps, yield_stress_vec);

  _scalar_plastic_strain = &declareProperty<Real>("scalar_plastic_strain");
  _scalar_plastic_strain_old = &declarePropertyOld<Real>("scalar_plastic_strain");
}

void
IsotropicTempDepHardening::computeStressInitialize(unsigned qp, Real effectiveTrialStress, const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT = dynamic_cast<const SymmIsotropicElasticityTensor*>(&elasticityTensor);
  if (!eT)
    mooseError("IsotropicTempDepHardening requires a SymmIsotropicElasticityTensor");

  _shear_modulus = eT->shearModulus();
  updateHardeningFunction(qp);
  computeYieldStress(qp);
  _yield_condition = effectiveTrialStress - _hardening_variable_old[qp] - _yield_stress;
  _hardening_variable[qp] = _hardening_variable_old[qp];
  _plastic_strain[qp] = _plastic_strain_old[qp];
}

void
IsotropicTempDepHardening::iterationFinalize(unsigned qp, Real scalar)
{
  if (_yield_condition > 0)
    _hardening_variable[qp] = computeHardeningVariable(qp, scalar);

  if (_scalar_plastic_strain)
  {
    (*_scalar_plastic_strain)[qp] = (*_scalar_plastic_strain_old)[qp] + scalar;
    _stress_old = _interp_hardening_function->sample((*_scalar_plastic_strain)[qp]);
  }
}

Real
IsotropicTempDepHardening::computeHardening(unsigned qp, Real /*scalar*/)
{
  const Real strain_old = (*_scalar_plastic_strain_old)[qp];
  //Should I use the current or old hardening function?
  Real  slope = _interp_hardening_function_old->sampleDerivative(strain_old);

  return slope;
}

Real
IsotropicTempDepHardening::computeHardeningVariable(unsigned qp, Real scalar)
{
  const Real strain = (*_scalar_plastic_strain)[qp];
  Real hardening_var = _interp_hardening_function->sample(strain + scalar) - _yield_stress;
//  Real hardening_var = _hardening_variable_old[qp] + (_hardening_slope * scalar);

  return hardening_var;
}

void
IsotropicTempDepHardening::computeYieldStress(unsigned qp)
{
  _yield_stress = _interp_yield_stress->sample(_temperature[qp]);

  if (_yield_stress <= 0)
    mooseError("Yield stress must be greater than zero");
}

void
IsotropicTempDepHardening::updateHardeningFunction(unsigned qp)
{
  _interp_hardening_function_old = _interp_hardening_function;

  Real temp = _temperature[qp];

  if (temp > _temp_dep_hardening_functions_temps[0] &&
      temp < _temp_dep_hardening_functions_temps.back())
  {
    //Find between which curves this temperature falls
    for (unsigned int i = 0; i < _temp_dep_hardening_functions_temps.size() - 1; ++i)
    {
      if (temp > _temp_dep_hardening_functions_temps[i] &&
          temp < _temp_dep_hardening_functions_temps[i+1])
      {
        unsigned int func_len = _temp_dep_hardening_functions[i]->functionSize();
        std::vector<Real> strain(func_len);
        std::vector<Real> stress(func_len);
        Real temp_lo = _temp_dep_hardening_functions_temps[i];
        Real temp_hi = _temp_dep_hardening_functions_temps[i+1];
        PiecewiseLinear * temp_lo_func = _temp_dep_hardening_functions[i];
        PiecewiseLinear * temp_hi_func = _temp_dep_hardening_functions[i+1];

        //Interpolate function at current temp
        for (unsigned int j = 0; j < func_len; ++j)
        {
          strain[j] = temp_lo_func->domain(j);
          Real stress_lo = temp_lo_func->range(j);
          Real stress_hi = temp_hi_func->range(j);
          stress[j] = stress_lo + (stress_hi - stress_lo) * ((temp - temp_lo) / (temp_hi - temp_lo));
        }
        _interp_hardening_function.reset(new LinearInterpolation(strain, stress));
      }
    }
  }

  else if (temp < _temp_dep_hardening_functions_temps[0])
  {
    std::vector<Real> strain;
    std::vector<Real> stress;

    for (unsigned int i = 0; i < _temp_dep_hardening_functions[0]->functionSize(); ++i)
    {
      strain.push_back(_temp_dep_hardening_functions[0]->domain(i));
      stress.push_back(_temp_dep_hardening_functions[0]->range(i));
    }
    _interp_hardening_function.reset(new LinearInterpolation(strain, stress));
  }

  else if (temp > _temp_dep_hardening_functions_temps.back())
  {
    std::vector<Real> strain;
    std::vector<Real> stress;

    for (unsigned int i = 0; i < _temp_dep_hardening_functions.back()->functionSize(); ++i)
    {
      strain.push_back(_temp_dep_hardening_functions.back()->domain(i));
      stress.push_back(_temp_dep_hardening_functions.back()->range(i));
    }
    _interp_hardening_function.reset(new LinearInterpolation(strain, stress));
  }
}
