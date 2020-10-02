//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KineticDisPreRateAux.h"

registerMooseObject("ChemicalReactionsApp", KineticDisPreRateAux);

InputParameters
KineticDisPreRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addCoupledVar("log_k", 0.0, "The equilibrium constant of the dissolution reaction");
  params.addRequiredParam<std::vector<Real>>("sto_v",
                                             "The stoichiometric coefficients of reactant species");
  params.addParam<Real>("r_area", 0.1, "Specific reactive surface area in m^2/L solution");
  params.addParam<Real>("ref_kconst", 6.456542e-8, "Kinetic rate constant in mol/m^2 s");
  params.addParam<Real>("e_act", 2.91e4, "Activation energy, J/mol");
  params.addParam<Real>("gas_const", 8.31434, "Gas constant, in J/mol K");
  params.addParam<Real>("ref_temp", 298.15, "Reference temperature, K");
  params.addCoupledVar("sys_temp", 298.15, "System temperature, K");
  params.addCoupledVar("v", "The list of reactant species");
  params.addClassDescription("Kinetic rate of secondary kinetic species");
  return params;
}

KineticDisPreRateAux::KineticDisPreRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(coupledValue("log_k")),
    _r_area(getParam<Real>("r_area")),
    _ref_kconst(getParam<Real>("ref_kconst")),
    _e_act(getParam<Real>("e_act")),
    _gas_const(getParam<Real>("gas_const")),
    _ref_temp(getParam<Real>("ref_temp")),
    _sys_temp(coupledValue("sys_temp")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _vals(coupledValues("v"))
{
  // Check that the number of stoichiometric coefficients is equal to the number
  // of reactant species
  if (_sto_v.size() != coupledComponents("v"))
    mooseError(
        "The number of stoichiometric coefficients in sto_v is not equal to the number of reactant "
        "species in ",
        _name);
}

Real
KineticDisPreRateAux::computeValue()
{
  const Real kconst =
      _ref_kconst * std::exp(_e_act * (1.0 / _ref_temp - 1.0 / _sys_temp[_qp]) / _gas_const);
  Real omega = 1.0;

  if (_vals.size() > 0)
  {
    for (unsigned int i = 0; i < _vals.size(); ++i)
    {
      if ((*_vals[i])[_qp] < 0.0)
        omega *= 0.0;
      else
        omega *= std::pow((*_vals[i])[_qp], _sto_v[i]);
    }
  }

  const Real saturation_SI = omega / std::pow(10.0, _log_k[_qp]);
  Real kinetic_rate = _r_area * kconst * (1.0 - saturation_SI);

  if (std::abs(kinetic_rate) <= 1.0e-12)
    kinetic_rate = 0.0;

  return -kinetic_rate;
}
