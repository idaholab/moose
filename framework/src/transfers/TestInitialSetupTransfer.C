/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "TestInitialSetupTransfer.h"

template<>
InputParameters validParams<TestInitialSetupTransfer>()
{
  InputParameters params = validParams<MultiAppProjectionTransfer>();
  return params;
}

TestInitialSetupTransfer::TestInitialSetupTransfer(const std::string & name, InputParameters parameters) :
    MultiAppProjectionTransfer(name, parameters)
{
}

TestInitialSetupTransfer::~TestInitialSetupTransfer()
{
}

void
TestInitialSetupTransfer::initialSetup()
{
  mooseError("TestInitialSetupTransfer terminated in initialSetup as expected!!");
}

void
TestInitialSetupTransfer::execute()
{
  MultiAppProjectionTransfer::execute();
  mooseError("TestInitialSetupTransfer should not even get to this point!!");
}

