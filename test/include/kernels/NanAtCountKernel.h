//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Kernel that generates NaN
 */
class NanAtCountKernel : public Kernel
{
public:
  static InputParameters validParams();

  NanAtCountKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  /// Compute this Kernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() override;
  /// Compute this Kernel's contribution to the residual
  virtual void computeResidual() override;

private:
  /// The residual count to nan at
  unsigned int _count_to_nan;

  /// Whether or not to print out the count
  bool _print_count;

  /// The current count
  unsigned int _count;
};
