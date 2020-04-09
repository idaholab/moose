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
 * Creates a cumulative sum of a post-processor value over a transient.
 *
 * This is useful, for example, for counting the total number of linear or
 * nonlinear iterations during a transient.
 */
class CumulativeValuePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  CumulativeValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  /// cumulative sum of the post-processor value
  Real _sum;

  /// cumulative sum of the post-processor value from the old time step */
  const PostprocessorValue & _sum_old;

  /// current post-processor value to be added to the cumulative sum
  const PostprocessorValue & _pps_value;
};
