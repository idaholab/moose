//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IterationCountConvergence.h"

/**
 * Takes a user-supplied vector of convergence statuses for each iteration.
 */
class SuppliedStatusConvergence : public IterationCountConvergence
{
public:
  static InputParameters validParams();

  SuppliedStatusConvergence(const InputParameters & parameters);

protected:
  virtual MooseConvergenceStatus checkConvergenceInner(unsigned int iter) override;

  /// Convergence status for each iteration
  const std::vector<int> & _convergence_statuses;
};
