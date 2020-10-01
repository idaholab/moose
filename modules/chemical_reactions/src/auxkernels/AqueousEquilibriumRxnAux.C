//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AqueousEquilibriumRxnAux.h"

registerMooseObject("ChemicalReactionsApp", AqueousEquilibriumRxnAux);

InputParameters
AqueousEquilibriumRxnAux::validParams()
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

AqueousEquilibriumRxnAux::AqueousEquilibriumRxnAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(coupledValue("log_k")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _gamma_eq(coupledValue("gamma")),
    _vals(coupledValues("v")),
    _gamma_v(isCoupled("gamma_v")
                 ? coupledValues("gamma_v") // have value
                 : std::vector<const VariableValue *>(coupledComponents("v"),
                                                      &coupledValue("gamma_v"))) // default
{
  const unsigned int n = coupledComponents("v");

  // Check that the correct number of stoichiometric coefficients have been provided
  if (_sto_v.size() != n)
    mooseError("The number of stoichiometric coefficients in sto_v is not equal to the number of "
               "coupled species in ",
               _name);

  // Check that the correct number of activity coefficients have been provided (if applicable)
  if (isCoupled("gamma_v"))
    if (coupledComponents("gamma_v") != n)
      mooseError("The number of activity coefficients in gamma_v is not equal to the number of "
                 "coupled species in ",
                 _name);
}

Real
AqueousEquilibriumRxnAux::computeValue()
{
  Real conc_product = 1.0;

  for (unsigned int i = 0; i < _vals.size(); ++i)
    conc_product *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

  mooseAssert(_gamma_eq[_qp] > 0.0, "Activity coefficient must be greater than zero");
  return std::pow(10.0, _log_k[_qp]) * conc_product / _gamma_eq[_qp];
}
