//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCoupledBEEquilibriumSub.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosCoupledBEEquilibriumSub);

InputParameters
KokkosCoupledBEEquilibriumSub::validParams()
{
  InputParameters params = TimeKernelValue::validParams();
  params.addParam<Real>(
      "weight", 1.0, "The weight of the equilibrium species in total concentration");
  params.addCoupledVar("log_k", 0.0, "The equilibrium constant of this equilibrium species");
  params.addParam<Real>(
      "sto_u",
      1.0,
      "The stoichiometric coefficient of the primary species this kernel operates on");
  params.addCoupledVar(
      "gamma_u", 1.0, "Activity coefficient of primary species that this kernel operates on");
  params.addParam<std::vector<Real>>(
      "sto_v", {}, "The stoichiometric coefficients of coupled primary species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addCoupledVar("gamma_eq", 1.0, "Activity coefficient of this equilibrium species");
  params.addCoupledVar("v", "Coupled primary species constituting the equilibrium species");
  params.addClassDescription("Derivative of equilibrium species concentration wrt time");
  return params;
}

KokkosCoupledBEEquilibriumSub::KokkosCoupledBEEquilibriumSub(const InputParameters & parameters)
  : TimeKernelValue(parameters),
    _weight(getParam<Real>("weight")),
    _log_k(kokkosCoupledValue("log_k")),
    _sto_u(getParam<Real>("sto_u")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _gamma_u(kokkosCoupledValue("gamma_u")),
    _gamma_u_old(kokkosCoupledValueOld("gamma_u")),
    _gamma_v_coupled(isCoupled("gamma_v")),
    _gamma_v(kokkosCoupledValues("gamma_v")),
    _gamma_v_old(kokkosCoupledValuesOld("gamma_v")),
    _gamma_eq(kokkosCoupledValue("gamma_eq")),
    _gamma_eq_old(kokkosCoupledValueOld("gamma_eq")),
    _porosity(getKokkosMaterialProperty<Real>("porosity")),
    _vars(coupledIndices("v")),
    _n(coupledComponents("v")),
    _v_vals(kokkosCoupledValues("v")),
    _v_vals_old(kokkosCoupledValuesOld("v")),
    _u_old(_var, Moose::OLD_SOLUTION_TAG)
{
  const unsigned int n = coupledComponents("v");
  // Check that the correct number of coupled values have been provided
  if (_sto_v.size() != n)
    mooseError("The number of stoichiometric coefficients in sto_v is not equal to the number of "
               "coupled species in ",
               _name);
  if (isCoupled("gamma_v"))
    if (coupledComponents("gamma_v") != n)
      mooseError("The number of activity coefficients in gamma_v is not equal to the number of "
                 "coupled species in ",
                 _name);

  for (const auto i : make_range(n))
    _var_to_index[_vars[i]] = i;
  _var_to_index.copyToDevice();
}
