//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IterationCountConvergence.h"

/**
 * Converges if multiple post-processors are all less than tolerances.
 */
class MultiPostprocessorConvergence : public IterationCountConvergence
{
public:
  static InputParameters validParams();

  MultiPostprocessorConvergence(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual MooseConvergenceStatus checkConvergenceInner(unsigned int iter) override;

protected:
  /// Returns the maximum description length
  unsigned int getMaxDescriptionLength() const;

  /// Generates a colored line for a tolerance comparison
  std::string comparisonLine(const std::string & description, const Real err, const Real tol) const;

  /// Postprocessor values
  std::vector<const PostprocessorValue *> _pp_values;
  /// Description of each Postprocessor
  std::vector<std::string> _descriptions;
  /// Tolerance for each Postprocessor
  const std::vector<Real> & _tolerances;

  /// Maximum description length
  unsigned int _max_desc_length;
};
