//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVSetupTester.h"
#include "FVFluxKernel.h"

/**
 * Zero-residual FV flux kernel that counts the setup methods it receives, for testing setup
 * dispatch.
 */
class FVSetupTestFluxKernel : public FVSetupTester<FVFluxKernel>
{
public:
  static InputParameters validParams();
  FVSetupTestFluxKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override { return 0; }
};
