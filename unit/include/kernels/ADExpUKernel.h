//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * Field kernel computing exp(u - 1) for the Keyes MSPIN unit test.
 */
class ADExpUKernel : public ADKernelValue
{
public:
  static InputParameters validParams();
  ADExpUKernel(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;
};
