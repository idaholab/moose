//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorSampler.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("StochasticToolsApp", VectorPostprocessorSampler);

InputParameters
VectorPostprocessorSampler::validParams()
{
  InputParameters params = Sampler::validParams();

  params.addRequiredParam<std::vector<ReporterName>>(
      "vectors_names", "The names of the vector postprocessors containing the column data.");

  params.addClassDescription("The sampler uses vector postprocessors as inputs.");
  return params;
}

VectorPostprocessorSampler::VectorPostprocessorSampler(const InputParameters & parameters)
  : Sampler(parameters), _reporter_names(getParam<std::vector<ReporterName>>("vectors_names"))
{
  _data.clear();
  if (!_reporter_names.empty())
    for (auto & vpp_vec : _reporter_names)
      _data.emplace_back(&getReporterValueByName<std::vector<Real>>(vpp_vec));

  // set a non-zero value for number of rows to avoid error in rankConfig, the actual value will be
  // set in executableSetup() and update via reinit()
  setNumberOfRows(1);
}

void
VectorPostprocessorSampler::executeSetUp()
{
  setNumberOfRows(_data[0]->size());
  setNumberOfCols(_data.size());
}

Real
VectorPostprocessorSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  // Checks to make sure that the row and column indices are not out of bounds
  mooseAssert(row_index < _data[0]->size(), "row_index cannot be out of bounds of the data.");
  mooseAssert(col_index < _data.size(), "col_index cannot be out of bounds of the data.");

  // Entering samples into the matrix
  return (*_data[col_index])[row_index];
}
