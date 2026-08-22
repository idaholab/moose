//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSetupTestDirichletBC.h"

registerMooseObject("MooseTestApp", FVSetupTestDirichletBC);

InputParameters
FVSetupTestDirichletBC::validParams()
{
  InputParameters params = FVSetupTester<FVDirichletBCBase>::validParams();
  params.addParam<Real>("value", 0, "The value to impose on the boundary");
  params.addClassDescription("FV Dirichlet boundary condition that reports the setup methods it "
                             "receives, for testing setup dispatch.");
  return params;
}

FVSetupTestDirichletBC::FVSetupTestDirichletBC(const InputParameters & params)
  : FVSetupTester<FVDirichletBCBase>(params), _value(getParam<Real>("value"))
{
}
