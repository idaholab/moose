//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSetupTestElementalKernel.h"

registerMooseObject("MooseTestApp", FVSetupTestElementalKernel);

InputParameters
FVSetupTestElementalKernel::validParams()
{
  InputParameters params = FVSetupTester<FVElementalKernel>::validParams();
  params.addClassDescription("Zero-residual FV elemental kernel that counts the setup methods it "
                             "receives, for testing setup dispatch.");
  return params;
}

FVSetupTestElementalKernel::FVSetupTestElementalKernel(const InputParameters & params)
  : FVSetupTester<FVElementalKernel>(params)
{
}
