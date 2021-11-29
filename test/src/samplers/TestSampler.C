//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "TestSampler.h"

registerMooseObject("MooseTestApp", TestSampler);

InputParameters
TestSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addParam<bool>("use_rand", false, "Use rand method for computeSample method.");
  params.addParam<dof_id_type>("num_rows", 14, "Number of rows.");
  params.addParam<dof_id_type>("num_cols", 8, "Number of columns.");

  MooseEnum error_tests(
      "call_set_number_of_rows call_set_number_of_cols call_set_number_of_seeds "
      "set_number_of_seeds_to_zero reinit_getGlobalSamples reinit_getLocalSamples "
      "reinit_getNextLocalRow reinit_getNumberOfRows reinit_getNumberOfCols "
      "reinit_getNumberOfLocalRows reinit_getLocalRowBegin reinit_getLocalRowEnd");
  params.addParam<MooseEnum>(
      "error_test", error_tests, "Options for making this class force errors.");
  return params;
}

TestSampler::TestSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _use_rand(getParam<bool>("use_rand")),
    _error_test(getParam<MooseEnum>("error_test"))
{
  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(getParam<dof_id_type>("num_cols"));
  if (_error_test == "set_number_of_seeds_to_zero")
    setNumberOfRandomSeeds(0);
}

void
TestSampler::executeSetUp()
{
  if (_error_test.isValid())
  {
    setNumberOfRows(getNumberOfRows() + 1);
    if (_error_test == "reinit_getGlobalSamples")
      getGlobalSamples();
    else if (_error_test == "reinit_getLocalSamples")
      getLocalSamples();
    else if (_error_test == "reinit_getNextLocalRow")
      getNextLocalRow();
    else if (_error_test == "reinit_getNumberOfRows")
      getNumberOfRows();
    else if (_error_test == "reinit_getNumberOfLocalRows")
      getNumberOfLocalRows();
    else if (_error_test == "reinit_getNumberOfCols")
      getNumberOfCols();
    else if (_error_test == "reinit_getLocalRowBegin")
      getLocalRowBegin();
    else if (_error_test == "reinit_getLocalRowEnd")
      getLocalRowEnd();
  }
}

Real
TestSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  if (_error_test == "call_set_number_of_rows")
    setNumberOfRows(1980);
  else if (_error_test == "call_set_number_of_cols")
    setNumberOfCols(1980);
  else if (_error_test == "call_set_number_of_seeds")
    setNumberOfRandomSeeds(1980);

  if (_use_rand)
    return getRand();
  else
    return ((row_index + 1) * 10) + col_index;
}
