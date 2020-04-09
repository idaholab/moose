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
 * Compares two post-processors and produces a boolean value
 *
 * This post-processor implements a number of different comparisons between
 * post-processor \c a and post-processor \c b:
 * \li <b>equals</b>: <tt>a == b</tt>?
 * \li <b>greater_than</b>: <tt>a > b</tt>?
 * \li <b>greater_than_equals</b>: <tt>a >= b</tt>?
 * \li <b>less_than</b>: <tt>a < b</tt>?
 * \li <b>less_than_equals</b>: <tt>a <= b</tt>?
 *
 * For all comparisons, a "fuzzy" comparison is made via the corresponding
 * function in \c MooseUtils.
 * If the comparison condition is true, then a value of 1 is output;
 * otherwise, a value of 0 is output.
 */
class PostprocessorComparison : public ComparisonPostprocessor
{
public:
  static InputParameters validParams();

  PostprocessorComparison(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// First post-processor to compare
  const PostprocessorValue & _value_a;
  /// Second post-processor to compare
  const PostprocessorValue & _value_b;

  /// The comparison value; 1 for true and 0 for false
  PostprocessorValue _comparison_value;
};
