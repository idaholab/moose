/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
  return params;
}

AqueousEquilibriumRxnAux::AqueousEquilibriumRxnAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _log_k(getParam<Real>("log_k")),
    _sto_v(getParam<std::vector<Real>>("sto_v"))
{
  int n = coupledComponents("v");
  _vals.resize(n);
  for (unsigned int i = 0; i < _vals.size(); ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
AqueousEquilibriumRxnAux::computeValue()
{
  Real conc_product = 1.0;

  for (unsigned int i = 0; i < _vals.size(); ++i)
    conc_product *= std::pow((*_vals[i])[_qp], _sto_v[i]);

  return std::pow(10.0, _log_k) * conc_product;
}
