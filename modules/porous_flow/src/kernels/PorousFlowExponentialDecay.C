//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowExponentialDecay.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowExponentialDecay);
registerMooseObject("PorousFlowApp", ADPorousFlowExponentialDecay);

template <bool is_ad>
InputParameters
PorousFlowExponentialDecayTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addCoupledVar("rate", 1.0, "Rate of exponential decay");
  params.addCoupledVar("reference", 0.0, "Reference value of the variable");
  params.addClassDescription("Residual = rate * (variable - reference).  Useful for modelling "
                             "exponential decay of a variable");
  return params;
}

template <bool is_ad>
PorousFlowExponentialDecayTempl<is_ad>::PorousFlowExponentialDecayTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _rate(this->template coupledGenericValue<is_ad>("rate")),
    _reference(this->template coupledGenericValue<is_ad>("reference"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowExponentialDecayTempl<is_ad>::computeQpResidual()
{
  return _test[_i][_qp] * _rate[_qp] * (_u[_qp] - _reference[_qp]);
}

template <bool is_ad>
Real
PorousFlowExponentialDecayTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
    return _test[_i][_qp] * _rate[_qp] * _phi[_j][_qp];
  return 0.0;
}

template class PorousFlowExponentialDecayTempl<false>;
template class PorousFlowExponentialDecayTempl<true>;
