//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  virtual Real computeQpJacobian() override;

  /// Scalar coefficient representing the relative amount consumed per unit time
  const Real & _rate;

  usingGenericKernelMembers;
};

typedef ReactionTempl<false> Reaction;
typedef ReactionTempl<true> ADReaction;
