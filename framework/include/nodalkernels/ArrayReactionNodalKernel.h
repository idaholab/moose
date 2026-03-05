//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericArrayNodalKernel.h"

/**
 * Represents a nodal reaction term equivalent to $a * u$
 */
template <bool is_ad>
class ArrayReactionNodalKernelTempl : public GenericArrayNodalKernel<is_ad>
{
public:
  static InputParameters validParams();

  ArrayReactionNodalKernelTempl(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(GenericRealEigenVector<is_ad> & residual) override;
  virtual void computeQpJacobian() override;

  /// rate coefficient
  const RealEigenVector & _coeff;

  usingGenericArrayNodalKernelMembers;
};

typedef ArrayReactionNodalKernelTempl<false> ArrayReactionNodalKernel;
typedef ArrayReactionNodalKernelTempl<true> ADArrayReactionNodalKernel;
