//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSetupTestInterfaceKernel.h"

registerMooseObject("MooseTestApp", FVSetupTestInterfaceKernel);

InputParameters
FVSetupTestInterfaceKernel::validParams()
{
  InputParameters params = FVSetupTester<FVInterfaceKernel>::validParams();
  params.addClassDescription("Zero-residual FV interface kernel that counts the setup methods it "
                             "receives, for testing setup dispatch.");
  return params;
}

FVSetupTestInterfaceKernel::FVSetupTestInterfaceKernel(const InputParameters & params)
  : FVSetupTester<FVInterfaceKernel>(params)
{
}
