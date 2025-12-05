//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"

/**
 * Base class for convergence criteria that checks a list of post-processor values.
 */
class MultiPostprocessorConvergence : public Convergence
{
public:
  static InputParameters validParams();

  MultiPostprocessorConvergence(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /// Returns a vector of tuples of description, error, and tolerance
  virtual std::vector<std::tuple<std::string, Real, Real>>
  getDescriptionErrorToleranceTuples() const = 0;
  /// Returns the minimum number of iterations required before convergence
  virtual unsigned int getMinimumIterations() const = 0;

  /// Returns the maximum description length
  unsigned int getMaxDescriptionLength() const;

  /// Generates a colored line for a tolerance comparison
  std::string comparisonLine(const std::string & description, Real err, Real tol) const;

  /// Maximum description length
  unsigned int _max_desc_length;
};
