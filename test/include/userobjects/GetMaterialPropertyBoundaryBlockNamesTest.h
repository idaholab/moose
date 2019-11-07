//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * A Postprocessor to test a call to GetMaterialPropertyBoundaryName() method.
 *
 * This class is for testing only, it will always produce an error.
 */
class GetMaterialPropertyBoundaryBlockNamesTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GetMaterialPropertyBoundaryBlockNamesTest(const InputParameters & parameters);
  virtual ~GetMaterialPropertyBoundaryBlockNamesTest(){};
  virtual void execute(){};
  virtual void initialize(){};
  virtual void finalize(){};
  virtual void initialSetup();

private:
  MooseEnum _test_type;

  template <typename T>
  void performTest(const std::vector<T> & retrieved_names);
};

template <typename T>
void
GetMaterialPropertyBoundaryBlockNamesTest::performTest(const std::vector<T> & retrieved_names)
{
  // Extract the expected names
  std::vector<std::string> expected_names = getParam<std::vector<std::string>>("expected_names");

  // Vectors must be same length
  if (retrieved_names.size() != expected_names.size())
    mooseError("TEST FAILED: The vectors of retrieved and expected names are of different lengths");

  // Test that the vectors are the same
  for (unsigned int i = 0; i < retrieved_names.size(); i++)
  {
    std::vector<std::string>::const_iterator it =
        std::find(expected_names.begin(), expected_names.end(), retrieved_names[i]);
    if (it == expected_names.end())
      mooseError("TEST FAILED: The retrieved name ",
                 retrieved_names[i],
                 " was not located in the list of expected names.");
  }

  // If you are here, you win
  mooseError("TEST PASSED: The retrieved names are the same as the names expected");
}
