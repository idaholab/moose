//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKineticDisPreRateAux.h"

registerKokkosAuxKernel("ChemicalReactionsApp", KokkosKineticDisPreRateAux);

InputParameters
KokkosKineticDisPreRateAux::validParams()
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

KokkosKineticDisPreRateAux::KokkosKineticDisPreRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(kokkosCoupledValue("log_k")),
    _r_area(getParam<Real>("r_area")),
    _ref_kconst(getParam<Real>("ref_kconst")),
    _e_act(getParam<Real>("e_act")),
    _gas_const(getParam<Real>("gas_const")),
    _ref_temp(getParam<Real>("ref_temp")),
    _sys_temp(kokkosCoupledValue("sys_temp")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _n(coupledComponents("v")),
    _vals(kokkosCoupledValues("v"))
{
  // Check that the number of stoichiometric coefficients is equal to the number
  // of reactant species
  if (_sto_v.size() != _n)
    mooseError(
        "The number of stoichiometric coefficients in sto_v is not equal to the number of reactant "
        "species in ",
        _name);
}
