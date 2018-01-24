//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AqueousEquilibriumRxnAux.h"

template <>
InputParameters
validParams<AqueousEquilibriumRxnAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("log_k", 0.0, "The equilibrium constant in dissociation form");
  params.addRequiredParam<std::vector<Real>>("sto_v",
                                             "The stoichiometric coefficient of reactants");
  params.addCoupledVar("v",
                       "The list of primary species participating in this equilibrium species");
  params.addCoupledVar("gamma", 1.0, "Activity coefficient of this secondary equilibrium species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addClassDescription("Concentration of secondary equilibrium species");
  return params;
}

AqueousEquilibriumRxnAux::AqueousEquilibriumRxnAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(getParam<Real>("log_k")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _gamma_eq(coupledValue("gamma"))
{
  const unsigned int n = coupledComponents("v");

  // Check that the correct number of stoichiometric coefficients have been provided
  if (_sto_v.size() != n)
    mooseError("The number of stoichiometric coefficients and coupled species must be equal in ",
               _name);

  // Check that the correct number of activity coefficients have been provided (if applicable)
  if (isCoupled("gamma_v"))
    if (coupled("gamma_v") != n)
      mooseError("The number of activity coefficients and coupled species must be equal in ",
                 _name);

  _vals.resize(n);
  _gamma_v.resize(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    // If gamma_v has been supplied, use those values, but if not, use the default value
    _gamma_v[i] = (isCoupled("gamma_v") ? &coupledValue("gamma_v", i) : &coupledValue("gamma_v"));
  }
}

Real
AqueousEquilibriumRxnAux::computeValue()
{
  Real conc_product = 1.0;

  for (unsigned int i = 0; i < _vals.size(); ++i)
    conc_product *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

  mooseAssert(_gamma_eq[_qp] > 0.0, "Activity coefficient must be greater than zero");
  return std::pow(10.0, _log_k) * conc_product / _gamma_eq[_qp];
}
