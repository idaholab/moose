//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestDynamicNumberOfSubAppsSampler.h"

registerMooseObject("StochasticToolsTestApp", TestDynamicNumberOfSubAppsSampler);

InputParameters
TestDynamicNumberOfSubAppsSampler::validParams()
{
  InputParameters params = MonteCarloSampler::validParams();
  MooseEnum errors("reinit_getGlobalSamples reinit_getLocalSamples reinit_getNextLocalRow "
                   "reinit_getNumberOfRows reinit_getNumberOfCols reinit_getNumberOfLocalRows "
                   "reinit_getLocalRowBegin reinit_getLocalRowEnd");
  params.addParam<MooseEnum>("error_check", errors, "Control triggering of errors for testing.");
  params.addParam<dof_id_type>("increment_rows", 1, "Add this number of rows at each execution.");
  return params;
}

TestDynamicNumberOfSubAppsSampler::TestDynamicNumberOfSubAppsSampler(
    const InputParameters & parameters)
  : MonteCarloSampler(parameters),
    _error_check(getParam<MooseEnum>("error_check")),
    _increment_rows(getParam<dof_id_type>("increment_rows"))
{
}

void
TestDynamicNumberOfSubAppsSampler::executeSetUp()
{
  setNumberOfRows(getNumberOfRows() + _increment_rows);

  if (_error_check == "reinit_getGlobalSamples")
    getGlobalSamples();
  else if (_error_check == "reinit_getLocalSamples")
    getLocalSamples();
  else if (_error_check == "reinit_getNextLocalRow")
    getNextLocalRow();
  else if (_error_check == "reinit_getNumberOfRows")
    getNumberOfRows();
  else if (_error_check == "reinit_getNumberOfLocalRows")
    getNumberOfLocalRows();
  else if (_error_check == "reinit_getNumberOfCols")
    getNumberOfCols();
  else if (_error_check == "reinit_getLocalRowBegin")
    getLocalRowBegin();
  else if (_error_check == "reinit_getLocalRowEnd")
    getLocalRowEnd();
}
