//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayNodalKernel.h"

/**
 * Represents a nodal reaction term equivalent to $a * u$
 */
class ArrayReactionNodalKernel : public ArrayNodalKernel
{
public:
  static InputParameters validParams();

  ArrayReactionNodalKernel(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeQpJacobian() override;

  /// rate coefficient
  const RealEigenVector & _coeff;
};
