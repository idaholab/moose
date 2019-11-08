//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// moose_test includes
#include "GetMaterialPropertyBoundaryBlockNamesTest.h"

registerMooseObject("MooseTestApp", GetMaterialPropertyBoundaryBlockNamesTest);

InputParameters
GetMaterialPropertyBoundaryBlockNamesTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::string>("property_name",
                                       "The name of the property to extract boundary names for");
  params.addRequiredParam<std::vector<std::string>>(
      "expected_names", "The names expected to be returned by getMaterialPropertyBoundaryNames()");
  MooseEnum test_type("block boundary");
  params.addRequiredParam<MooseEnum>(
      "test_type", test_type, "The type of test to execute (block | boundary)");

  return params;
}

GetMaterialPropertyBoundaryBlockNamesTest::GetMaterialPropertyBoundaryBlockNamesTest(
    const InputParameters & parameters)
  : GeneralUserObject(parameters), _test_type(getParam<MooseEnum>("test_type"))
{
}

void
GetMaterialPropertyBoundaryBlockNamesTest::initialSetup()
{
  // Perform the desired boundary or block testing
  if (_test_type == "boundary")
    performTest<BoundaryName>(
        getMaterialPropertyBoundaryNames(getParam<std::string>("property_name")));
  else
    performTest<SubdomainName>(
        getMaterialPropertyBlockNames(getParam<std::string>("property_name")));
}
