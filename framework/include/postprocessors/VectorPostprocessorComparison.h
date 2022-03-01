//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComparisonPostprocessor.h"

/**
 * Compares two vector post-processors of equal size and produces a boolean value
 *
 * This post-processor implements a number of different comparisons between
 * vector post-processor \c a and vector post-processor \c b:
 * \li <b>equals</b>: <tt>a == b</tt>?
 * \li <b>greater_than</b>: <tt>a > b</tt>?
 * \li <b>greater_than_equals</b>: <tt>a >= b</tt>?
 * \li <b>less_than</b>: <tt>a < b</tt>?
 * \li <b>less_than_equals</b>: <tt>a <= b</tt>?
 *
 * For all comparisons, a "fuzzy" comparison is made via the corresponding
 * function in \c MooseUtils.
 * If the comparison condition is true for all elements of the vector post-
 * processors, then a value of 1 is output; otherwise, a value of 0 is
 * output.
 */
class VectorPostprocessorComparison : public ComparisonPostprocessor
{
public:
  static InputParameters validParams();

  VectorPostprocessorComparison(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// Values of the first vector post-processor to compare
  const VectorPostprocessorValue & _values_a;
  /// Values of the second vector post-processor to compare
  const VectorPostprocessorValue & _values_b;

  /// The comparison value; 1 for all true and 0 for at least one false
  PostprocessorValue _comparison_value;
};
