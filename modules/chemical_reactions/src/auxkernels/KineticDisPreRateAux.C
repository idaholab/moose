/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KineticDisPreRateAux.h"

template <>
InputParameters
validParams<KineticDisPreRateAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("log_k", 0.0, "The equilibrium constant of the dissolution reaction");
  params.addRequiredParam<std::vector<Real>>("sto_v",
                                             "The stoichiometric coefficients of reactant species");
  params.addParam<Real>("r_area", 0.1, "Specific reactive surface area in m^2/L solution");
  params.addParam<Real>("ref_kconst", 6.456542e-8, "Kinetic rate constant in mol/m^2 s");
  params.addParam<Real>("e_act", 2.91e4, "Activation energy, J/mol");
  params.addParam<Real>("gas_const", 8.31434, "Gas constant, in J/mol K");
  params.addParam<Real>("ref_temp", 298.15, "Reference temperature, K");
  params.addParam<Real>("sys_temp", 298.15, "System temperature, K");
  params.addCoupledVar("v", "The list of reactant species");
  return params;
}

KineticDisPreRateAux::KineticDisPreRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(getParam<Real>("log_k")),
    _r_area(getParam<Real>("r_area")),
    _ref_kconst(getParam<Real>("ref_kconst")),
    _e_act(getParam<Real>("e_act")),
    _gas_const(getParam<Real>("gas_const")),
    _ref_temp(getParam<Real>("ref_temp")),
    _sys_temp(getParam<Real>("sys_temp")),
    _sto_v(getParam<std::vector<Real>>("sto_v"))
{
  const unsigned int n = coupledComponents("v");
  _vals.resize(n);
  for (unsigned int i = 0; i < n; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
KineticDisPreRateAux::computeValue()
{
  const Real kconst =
      _ref_kconst * std::exp(-_e_act * (1.0 / _ref_temp - 1.0 / _sys_temp) / _gas_const);
  Real omega = 1.0;

  if (_vals.size())
  {
    for (unsigned int i = 0; i < _vals.size(); ++i)
    {
      if ((*_vals[i])[_qp] < 0.0)
        omega *= std::pow(0.0, _sto_v[i]);
      else
        omega *= std::pow((*_vals[i])[_qp], _sto_v[i]);
    }
  }

  const Real saturation_SI = omega / std::pow(10.0, _log_k);
  Real kinetic_rate = _r_area * kconst * (1.0 - saturation_SI);

  if (std::abs(kinetic_rate) <= 1.0e-12)
    kinetic_rate = 0.0;

  return -kinetic_rate;
}
