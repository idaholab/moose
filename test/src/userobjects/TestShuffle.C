//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*repl
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestShuffle.h"
#include "MooseRandom.h"
#include "Shuffle.h"

registerMooseObject("MooseTestApp", TestShuffle);

InputParameters
TestShuffle::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  MooseEnum test_type("swap shuffle resample");
  params.addParam<MooseEnum>("test_type", test_type, "The type of test to perform.");
  return params;
}

TestShuffle::TestShuffle(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _data(declareValueByName<std::vector<int>, ReporterGatherContext>("data"))
{
}

void
TestShuffle::execute()
{
  // Create a vector to test against
  std::vector<int> vec;
  if (n_processors() == 1)
    vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  else if (n_processors() == 2)
  {
    if (processor_id() == 0)
      vec = {0, 1, 2, 3};
    else
      vec = {4, 5, 6, 7, 8, 9};
  }
  else if (n_processors() == 3)
  {
    if (processor_id() == 0)
      vec = {0, 1, 2};
    else if (processor_id() == 1)
      vec = {3};
    else
      vec = {4, 5, 6, 7, 8, 9};
  }

  else
    mooseError("This test object only works with 1, 2, or 3 processors.");

  const auto test_type = getParam<MooseEnum>("test_type");
  if (test_type == "swap")
  {
    MooseUtils::swap(vec, 1, 2, _communicator); // both on 0
    MooseUtils::swap(vec, 8, 7, _communicator); // both on 1
    MooseUtils::swap(vec, 3, 9, _communicator); // 0 -> 1
    MooseUtils::swap(vec, 5, 0, _communicator); // 1 -> 0
  }
  else if (test_type == "shuffle")
  {
    MooseRandom generator;
    generator.seed(0, 1980);
    generator.saveState();
    MooseUtils::shuffle<int>(vec, generator, _communicator);
  }
  else if (test_type == "resample")
  {
    MooseRandom generator;
    generator.seed(0, 1980);
    generator.saveState();
    vec = MooseUtils::resample<int>(vec, generator, _communicator);
  }

  // Store the vector as ReporterValue, it is automatically gathered
  _data = vec;
}
