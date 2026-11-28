//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Kernel computing lambda * u^2 for the Keyes MSPIN unit test.
 */
class ADLambdaU2 : public ADKernel
{
public:
  static InputParameters validParams();
  ADLambdaU2(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const Real _lambda;
};
