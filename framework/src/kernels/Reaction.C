//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Reaction.h"

registerMooseObject("MooseApp", Reaction);
registerMooseObject("MooseApp", ADReaction);

template <bool is_ad>
InputParameters
ReactionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term with weak form $(\\psi_i, \\lambda u_h)$.");
  params.addParam<Real>(
      "rate", 1.0, "The $(\\lambda)$ multiplier, the relative amount consumed per unit time.");
  params.declareControllable("rate");
  return params;
}

template <bool is_ad>
ReactionTempl<is_ad>::ReactionTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters), _rate(this->template getParam<Real>("rate"))
{
}

template <bool is_ad>
GenericReal<is_ad>
ReactionTempl<is_ad>::computeQpResidual()
{
  return _test[_i][_qp] * _rate * _u[_qp];
}

template <bool is_ad>
Real
ReactionTempl<is_ad>::computeQpJacobian()
{
  // This function will never be called for the AD version. But because C++ does
  // not support an optional function declaration based on a template parameter,
  // we must keep this template for all cases.
  mooseAssert(!is_ad,
              "In ADReaction, computeQpJacobian should not be called. Check computeJacobian "
              "implementation.");
  return _test[_i][_qp] * _rate * _phi[_j][_qp];
}

template class ReactionTempl<false>;
template class ReactionTempl<true>;
