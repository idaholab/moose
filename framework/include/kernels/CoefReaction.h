//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Reaction.h"

template <bool is_ad>
class CoefReactionTempl : public ReactionTempl<is_ad>
{
public:
  static InputParameters validParams();

  CoefReactionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  Real computeQpJacobian() override;

  /// input parameter multiplied by the reaction kernel
  const Real _coef;

  using ReactionTempl<is_ad>::_test;
  using ReactionTempl<is_ad>::_i;
  using ReactionTempl<is_ad>::_qp;
  using ReactionTempl<is_ad>::_rate;
  using ReactionTempl<is_ad>::_phi;
  using ReactionTempl<is_ad>::_j;
};

typedef CoefReactionTempl<false> CoefReaction;
typedef CoefReactionTempl<true> ADCoefReaction;
