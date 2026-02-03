//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefReaction.h"

registerMooseObjectReplaced("MooseApp", CoefReaction, "01/01/2027 00:00", Reaction);
registerMooseObjectReplaced("MooseApp", ADCoefReaction, "01/21/2027 00:00", ADReaction);

template <bool is_ad>
InputParameters
CoefReactionTempl<is_ad>::validParams()
{
  InputParameters params = ReactionTempl<is_ad>::validParams();
  params.addClassDescription("Implements the residual term (p*u, test)");
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  return params;
}

template <bool is_ad>
CoefReactionTempl<is_ad>::CoefReactionTempl(const InputParameters & parameters)
  : ReactionTempl<is_ad>(parameters), _coef(this->template getParam<Real>("coefficient"))
{
}

template <bool is_ad>
GenericReal<is_ad>
CoefReactionTempl<is_ad>::computeQpResidual()
{
  return _coef * ReactionTempl<is_ad>::computeQpResidual();
}

template <bool is_ad>
Real
CoefReactionTempl<is_ad>::computeQpJacobian()
{
  mooseAssert(!is_ad,
              "In ADCoefReaction, computeQpJacobian should not be called. Check computeJacobian "
              "implementation.");
  return _coef * ReactionTempl<is_ad>::computeQpJacobian();
}

template class CoefReactionTempl<false>;
template class CoefReactionTempl<true>;
