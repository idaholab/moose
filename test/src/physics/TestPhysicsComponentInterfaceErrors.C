//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPhysicsComponentInterfaceErrors.h"

registerPhysicsBaseTasks("MooseTestApp", TestPhysicsComponentInterfaceErrors);
registerMooseAction("MooseTestApp", TestPhysicsComponentInterfaceErrors, "add_ic");
registerMooseAction("MooseTestApp", TestPhysicsComponentInterfaceErrors, "add_bc");
registerMooseAction("MooseTestApp", TestPhysicsComponentInterfaceErrors, "add_bc");

InputParameters
TestPhysicsComponentInterfaceErrors::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  return params;
}

TestPhysicsComponentInterfaceErrors::TestPhysicsComponentInterfaceErrors(
    const InputParameters & parameters)
  : PhysicsBase(parameters), PhysicsComponentInterface(parameters)
{
  // Keep track of a variable
  saveSolverVariableName("test_variable");
}
