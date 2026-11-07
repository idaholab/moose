//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericScalarKernel.h"

/**
 * Implements a simple consuming reaction term for a scalar variable: rate * u.
 */
template <bool is_ad>
class ScalarReactionTempl : public GenericScalarKernel<is_ad>
{
public:
  static InputParameters validParams();
  ScalarReactionTempl(const InputParameters & parameters);
  virtual void reinit() override {}

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const Real _rate;

  usingGenericScalarKernelMembers;
};

typedef ScalarReactionTempl<false> ScalarReaction;
typedef ScalarReactionTempl<true> ADScalarReaction;
