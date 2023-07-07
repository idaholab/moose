//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorSampler.h"

registerMooseObject("StochasticToolsApp", VectorPostprocessorSampler);

InputParameters
VectorPostprocessorSampler::validParams()
{
  InputParameters params = Sampler::validParams();

  params.addRequiredParam<std::vector<ReporterName>>(
      "vectors_names",
      "The names of the vector-postprocessors and/or vector reporter values containing the column "
      "data.");

  params.addClassDescription("The sampler uses vector postprocessors as inputs.");
  return params;
}

VectorPostprocessorSampler::VectorPostprocessorSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  for (auto & vpp_vec : getParam<std::vector<ReporterName>>("vectors_names"))
    _data.emplace_back(&getReporterValueByName<std::vector<Real>>(vpp_vec));

  // set a non-zero value for number of rows to avoid error in rankConfig, the actual value will be
  // set in executableSetup() and update via reinit()
  setNumberOfRows(1);
  setNumberOfCols(_data.size());
}

void
VectorPostprocessorSampler::executeSetUp()
{
  const auto vsize = _data[0]->size();
  for (const auto & vec : _data)
    if (vec->size() != vsize)
      paramError("vector_names", "Vectors must all be the same size.");

  setNumberOfRows(vsize);
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
