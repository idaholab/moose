//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericNodalKernel.h"

/**
 * Represents a nodal reaction term equivalent to $a * u$
 */
template <bool is_ad>
class ReactionNodalKernelTempl : public GenericNodalKernel<is_ad>
{
public:
  static InputParameters validParams();

  ReactionNodalKernelTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// An optional input-file supplied rate coefficient
  const Real _coeff;

  usingGenericNodalKernelMembers;
};

typedef ReactionNodalKernelTempl<false> ReactionNodalKernel;
typedef ReactionNodalKernelTempl<true> ADReactionNodalKernel;
