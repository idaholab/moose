//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosEquilibriumReactionFlux.h"
#include "KokkosKernelGrad.h"
#include "KokkosKernelValue.h"

template <typename Base>
InputParameters
KokkosEquilibriumReactionFlux<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.addParam<Real>(
      "weight",
      1.0,
      "Weight of equilibrium species concentration in the primary species concentration");
  params.addCoupledVar(
      "log_k", 0.0, "Equilibrium constant of the equilibrium reaction in dissociation form");
  params.addParam<Real>("sto_u",
                        1.0,
                        "Stoichiometric coef of the primary species this kernel "
                        "operates on in the equilibrium reaction");
  params.addCoupledVar(
      "gamma_u", 1.0, "Activity coefficient of primary species that this kernel operates on");
  params.addParam<std::vector<Real>>(
      "sto_v", {}, "The stoichiometric coefficients of coupled primary species");
  params.addCoupledVar("v", "List of coupled primary species in this equilibrium species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addCoupledVar("gamma_eq", 1.0, "Activity coefficient of this equilibrium species");
  return params;
}

template <typename Base>
KokkosEquilibriumReactionFlux<Base>::KokkosEquilibriumReactionFlux(
    const InputParameters & parameters)
  : Base(parameters),
    _weight(this->template getParam<Real>("weight")),
    _log_k(this->kokkosCoupledValue("log_k")),
    _sto_u(this->template getParam<Real>("sto_u")),
    _sto_v(this->template getParam<std::vector<Real>>("sto_v")),
    _vars(this->coupledIndices("v")),
    _n(this->coupledComponents("v")),
    _vals(this->kokkosCoupledValues("v")),
    _grad_vals(this->kokkosCoupledGradients("v")),
    _gamma_u(this->kokkosCoupledValue("gamma_u")),
    _gamma_v_coupled(this->isCoupled("gamma_v")),
    _gamma_v(this->kokkosCoupledValues("gamma_v")),
    _gamma_eq(this->kokkosCoupledValue("gamma_eq"))
{
  // Check that the correct number of coupled values have been provided
  if (_sto_v.size() != _n)
    this->mooseError("The number of stoichiometric coefficients in sto_v is not equal to the "
                     "number of coupled species in ",
                     this->name());
  if (_gamma_v_coupled)
    if (this->coupledComponents("gamma_v") != _n)
      this->mooseError("The number of activity coefficients in gamma_v is not equal to the number "
                       "of coupled species in ",
                       this->name());

  for (const auto i : make_range(_n))
    _var_to_index[_vars[i]] = i;
  _var_to_index.copyToDevice();
}

template class KokkosEquilibriumReactionFlux<Moose::Kokkos::KernelGrad>;
template class KokkosEquilibriumReactionFlux<Moose::Kokkos::KernelValue>;
