//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarReaction.h"

registerMooseObject("MooseApp", ScalarReaction);
registerMooseObject("MooseApp", ADScalarReaction);

template <bool is_ad>
InputParameters
ScalarReactionTempl<is_ad>::validParams()
{
  InputParameters params = GenericScalarKernel<is_ad>::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term for a scalar variable: $(\\lambda u)$.");
  params.addParam<Real>(
      "rate", 1.0, "The $(\\lambda)$ multiplier, the relative amount consumed per unit time.");
  params.declareControllable("rate");
  return params;
}

template <bool is_ad>
ScalarReactionTempl<is_ad>::ScalarReactionTempl(const InputParameters & parameters)
  : GenericScalarKernel<is_ad>(parameters), _rate(this->template getParam<Real>("rate"))
{
}

template <bool is_ad>
GenericReal<is_ad>
ScalarReactionTempl<is_ad>::computeQpResidual()
{
  return _rate * _u[_i];
}

template <bool is_ad>
Real
ScalarReactionTempl<is_ad>::computeQpJacobian()
{
  mooseAssert(!is_ad,
              "In ADScalarReaction, computeQpJacobian should not be called. Check "
              "computeJacobian implementation.");
  return (_i == _j) ? _rate : 0.0;
}

template class ScalarReactionTempl<false>;
template class ScalarReactionTempl<true>;
