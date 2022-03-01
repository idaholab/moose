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
 * Computes a linear combination between an arbitrary number of post-processors
 *
 * This computes a linear combination of post-processors \f$x_i\f$:
 * \f[
 *   y = \sum\limits_i c_i x_i + b \,,
 * \f]
 * where \f$c_i\f$ is the combination coefficient for \f$x_i\f$ and \f$b\f$
 * is an additional value to add to the sum.
 */
class LinearCombinationPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  LinearCombinationPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// Number of post-processors in linear combination
  const unsigned int _n_pp;
  /// List of linear combination coefficients for each post-processor value
  const std::vector<Real> & _pp_coefs;
  /// Additional value to add to sum
  const Real _b_value;

  /// List of post-processor values
  std::vector<const PostprocessorValue *> _pp_values;
};
