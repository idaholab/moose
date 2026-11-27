//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAqueousEquilibriumRxnAux.h"

registerKokkosAuxKernel("ChemicalReactionsApp", KokkosAqueousEquilibriumRxnAux);

InputParameters
KokkosAqueousEquilibriumRxnAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addCoupledVar("log_k", 0.0, "The equilibrium constant in dissociation form");
  params.addRequiredParam<std::vector<Real>>("sto_v",
                                             "The stoichiometric coefficient of reactants");
  params.addRequiredCoupledVar(
      "v", "The list of primary species participating in this equilibrium species");
  params.addCoupledVar("gamma", 1.0, "Activity coefficient of this secondary equilibrium species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addClassDescription("Concentration of secondary equilibrium species");
  return params;
}

KokkosAqueousEquilibriumRxnAux::KokkosAqueousEquilibriumRxnAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(kokkosCoupledValue("log_k")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _gamma_eq(kokkosCoupledValue("gamma")),
    _n(coupledComponents("v")),
    _vals(kokkosCoupledValues("v")),
    _gamma_v_coupled(isCoupled("gamma_v")),
    _gamma_v(kokkosCoupledValues("gamma_v"))
{
  // Check that the correct number of stoichiometric coefficients have been provided
  if (_sto_v.size() != _n)
    mooseError("The number of stoichiometric coefficients in sto_v is not equal to the number of "
               "coupled species in ",
               _name);
  // Check that the correct number of activity coefficients have been provided (if applicable)
  if (_gamma_v_coupled)
    if (coupledComponents("gamma_v") != _n)
      mooseError("The number of activity coefficients in gamma_v is not equal to the number of "
                 "coupled species in ",
                 _name);
}
