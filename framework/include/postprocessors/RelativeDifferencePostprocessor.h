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
 * Computes the absolute value of the relative difference between 2
 * post-processor values.
 *
 * This post-processor computes the absolute value of the relative difference
 * between 2 post-processor values:
 * \f[
 *   y = \left| \frac{x_1 - x_2}{x_2} \right| \,,
 * \f]
 * where \f$x_1\f$ and \f$x_2\f$ are the 2 post-processor values. Note that
 * \f$x_2\f$ is used as the base for the relative difference. If
 * \f$x_2 \approx 0\f$, then the absolute difference is used instead to prevent
 * division by zero:
 * \f[
 *   y = \left| x_1 - x_2 \right| \,.
 * \f]
 */
class RelativeDifferencePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RelativeDifferencePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// first post-processor value
  const PostprocessorValue & _value1;
  /// second post-processor value, used as base in relative difference
  const PostprocessorValue & _value2;
};
