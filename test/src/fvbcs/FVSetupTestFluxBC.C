//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSetupTestFluxBC.h"

registerMooseObject("MooseTestApp", FVSetupTestFluxBC);

InputParameters
FVSetupTestFluxBC::validParams()
{
  InputParameters params = FVSetupTester<FVFluxBC>::validParams();
  params.addClassDescription("Zero-residual FV flux boundary condition that counts the setup "
                             "methods it receives, for testing setup dispatch.");
  return params;
}

FVSetupTestFluxBC::FVSetupTestFluxBC(const InputParameters & params)
  : FVSetupTester<FVFluxBC>(params)
{
}
