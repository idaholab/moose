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

#include "TestBoundaryRestrictableAssert.h"

template<>
InputParameters validParams<TestBoundaryRestrictableAssert>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addParam<bool>("test_invalid", false, "Set to true for test for the Moose::INVALID_BOUNDARY_ID");
  return params;
}

TestBoundaryRestrictableAssert::TestBoundaryRestrictableAssert(const std::string & name, InputParameters parameters) :
  SideUserObject(name, parameters)
{
  if (getParam<bool>("test_invalid"))
  {
    if (_current_boundary_id == Moose::INVALID_BOUNDARY_ID)
      mooseError("Invalid boundary id test passed");
    else
      mooseError("Invalid boundary id test failed");
  }
}


void
TestBoundaryRestrictableAssert::execute()
{
  if (_current_boundary_id == 2)
    mooseError("Valid boundary id test passed");
  else
    mooseError("Valid boundary id test failed");
}
