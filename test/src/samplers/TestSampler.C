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

template <>
InputParameters
validParams<TestSampler>()
{
  InputParameters params = validParams<Sampler>();
  params.addParam<bool>("use_rand", false, "Use rand method for computeSample method.");
  params.addParam<dof_id_type>("num_rows", 14, "Number of rows.");
  params.addParam<dof_id_type>("num_cols", 8, "Number of columns.");
  return params;
}

TestSampler::TestSampler(const InputParameters & parameters)
  : Sampler(parameters), _use_rand(getParam<bool>("use_rand"))
{
  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(getParam<dof_id_type>("num_cols"));
}

Real
TestSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  if (_use_rand)
    return rand();
  else
    return ((row_index + 1) * 10) + col_index;
}
