//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

/**
 *  Implements a simple consuming reaction term with weak form $(\\psi_i, \\lambda u_h)$.
 */
template <bool is_ad>
class ReactionTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  ReactionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Scalar coefficient representing the relative amount consumed per unit time
  const Real & _rate;

  usingGenericKernelMembers;
};

class Reaction : public ReactionTempl<false>
{
public:
  static InputParameters validParams();

  Reaction(const InputParameters & parameters);

  using ReactionTempl<false>::ReactionTempl;

protected:
  virtual Real computeQpJacobian() override;
};

typedef ReactionTempl<true> ADReaction;
