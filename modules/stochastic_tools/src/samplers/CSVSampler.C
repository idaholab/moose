//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSVSampler.h"
#include "DelimitedFileReader.h"

registerMooseObject("StochasticToolsApp", CSVSampler);

InputParameters
CSVSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Sampler that reads samples from CSV file.");
  params.addRequiredParam<FileName>("samples_file",
                                    "Name of the CSV file that contains the samples matrix.");
  params.addParam<std::vector<dof_id_type>>(
      "column_indices",
      "Column indices in the CSV file to be sampled from. Number of indices here will be the same "
      "as the number of columns per matrix.");
  return params;
}

CSVSampler::CSVSampler(const InputParameters & parameters)
  : Sampler(parameters), _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  // Reading the samples file and getting data
  MooseUtils::DelimitedFileReader reader(getParam<FileName>("samples_file"), &_communicator);
  reader.read();
  _data = reader.getData();

  // If indices are not provided, all of the data will be read
  if (!isParamValid("column_indices"))
  {
    // _indices.resize(_data.size());
    for (unsigned int i = 0; i < _data.size(); i++)
      _indices.push_back(i);
  }
  else
  // If  indices are provided, check that they are all smaller than number of
  // columns in the data
  {
    _indices = getParam<std::vector<dof_id_type>>("column_indices");
    for (unsigned int i = 0; i < _indices.size(); i++)
    {
      if (_indices[i] >= _data.size())
        mooseError("In ",
                   _name,
                   ": column index, ",
                   _indices[i],
                   " is larger than the number of columns in the samples file.");
    }
  }

  setNumberOfRows(_data[0].size());
  setNumberOfCols(_indices.size());
}

Real
CSVSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  // Checks to make sure that the row and column indices are not out of bounds
  mooseAssert(row_index < _data[0].size(), "row_index cannot be out of bounds of the data file.");
  mooseAssert(col_index < _indices.size(), "col_index cannot be out of bounds of the provided column indices.")

  TIME_SECTION(_perf_compute_sample);
  return _data[_indices[col_index]][row_index]; // entering samples into the matrix
}
