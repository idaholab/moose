//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

namespace libMesh
{
template <typename>
class NumericVector;
}

/**
 * Computes the sum of components of the requested tagged vector
 */
class TagVectorSum : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TagVectorSum(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() const override;

protected:
  /// The vector we will take the sum of
  const NumericVector<Number> & _vec;
  /// The vector sum
  Number _sum;
};
