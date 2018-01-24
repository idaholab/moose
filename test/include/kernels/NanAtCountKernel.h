//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NANATCOUNTKERNEL_H
#define NANATCOUNTKERNEL_H

#include "Kernel.h"

// Forward Declaration
class NanAtCountKernel;

template <>
InputParameters validParams<NanAtCountKernel>();

/**
 * Kernel that generates NaN
 */
class NanAtCountKernel : public Kernel
{
public:
  NanAtCountKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

private:
  /// The residual count to nan at
  unsigned int _count_to_nan;

  /// Whether or not to print out the count
  bool _print_count;

  /// The current count
  unsigned int _count;
};

#endif // NANATCOUNTKERNEL_H
