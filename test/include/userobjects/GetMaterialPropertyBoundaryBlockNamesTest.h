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

#ifndef GETMATERIALPROPERTYBOUNDARYNAMESTEST_H
#define GETMATERIALPROPERTYBOUNDARYNAMESTEST_H

#include "GeneralUserObject.h"

class GetMaterialPropertyBoundaryBlockNamesTest;

template <>
InputParameters validParams<GetMaterialPropertyBoundaryBlockNamesTest>();

/**
 * A Postprocessor to test a call to GetMaterialPropertyBoundaryName() method.
 *
 * This class is for testing only, it will always produce an error.
 */
class GetMaterialPropertyBoundaryBlockNamesTest : public GeneralUserObject
{
public:
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

#endif /* GETMATERIALPROPERTYBOUNDARYNAMESTEST_H */
