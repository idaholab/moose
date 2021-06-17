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
  params.addParam<dof_id_type>("increment_rows", 1, "Add this number of rows at each execution.");
  return params;
}

TestDynamicNumberOfSubAppsSampler::TestDynamicNumberOfSubAppsSampler(
    const InputParameters & parameters)
  : MonteCarloSampler(parameters), _increment_rows(getParam<dof_id_type>("increment_rows"))
{
}

void
TestDynamicNumberOfSubAppsSampler::executeSetUp()
{
  setNumberOfRows(getNumberOfRows() + _increment_rows);
}
