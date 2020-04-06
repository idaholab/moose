//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Base class for comparing quantities and producing a boolean value
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
 */
class ComparisonPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ComparisonPostprocessor(const InputParameters & parameters);

protected:
  /**
   * Performs the selected comparison on the two values
   *
   * @param[in] a   First value in the comparison
   * @param[in] b   Second value in the comparison
   *
   * @return Boolean value for the comparison being true
   */
  bool comparisonIsTrue(const Real & a, const Real & b) const;

  /// Comparison type
  enum class ComparisonType
  {
    EQUALS,
    GREATER_THAN_EQUALS,
    LESS_THAN_EQUALS,
    GREATER_THAN,
    LESS_THAN
  };

  /// Type of comparison to perform
  const ComparisonType _comparison_type;

  /// Absolute tolerance for "fuzzy" comparisons
  const Real _absolute_tolerance;
};
