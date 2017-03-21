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

// moose_test includes
#include "GetMaterialPropertyBoundaryBlockNamesTest.h"

template <>
InputParameters
validParams<GetMaterialPropertyBoundaryBlockNamesTest>()
{
  InputParameters params = validParams<GeneralUserObject>();
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
