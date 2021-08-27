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
      "Column indices in the CSV file to be sampled from. Number of indices here "
      "will be the same as the number of columns per matrix.");
  params.addParam<std::vector<std::string>>(
      "column_names",
      "Column names in the CSV file to be sampled from. Number of columns names "
      "here will be the same as the number of columns per matrix.");
  return params;
}

CSVSampler::CSVSampler(const InputParameters & parameters) : Sampler(parameters)
{
  // If indices or names are not provided, all of the data will be read and the
  // matrix will be the same as the contents of the data file
  if (!isParamValid("column_indices") && !isParamValid("column_names"))
  {
    // Reading the samples file and getting data
    MooseUtils::DelimitedFileReader reader(getParam<FileName>("samples_file"), &_communicator);
    reader.read();
    _data = reader.getData();
  }
  // Both column indices and names cannot be provided
  else if (isParamValid("column_indices") && isParamValid("column_names"))
    mooseError("In sampler, ",
               _name,
               ": Please provide either column_indices or column_names but not both.");
  // If only column indices is provided, generate the matrix accordingly and
  // store it in _data
  else if (isParamValid("column_indices"))
  {
    std::vector<dof_id_type> indices = getParam<std::vector<dof_id_type>>("column_indices");
    for (unsigned int i = 0; i < indices.size(); i++)
    {
      // Reading the samples file and getting data
      MooseUtils::DelimitedFileReader reader(getParam<FileName>("samples_file"), &_communicator);
      reader.read();
      _data.push_back(reader.getData(indices[i]));
    }
  }
  // If column names are provided, generate the matrix accordingly and store it
  // in _data
  else
  {
    std::vector<std::string> names = getParam<std::vector<std::string>>("column_names");
    for (unsigned int i = 0; i < names.size(); i++)
    {
      // Reading the samples file and getting data
      MooseUtils::DelimitedFileReader reader(getParam<FileName>("samples_file"), &_communicator);
      reader.read();
      _data.push_back(reader.getData(names[i]));
    }
  }

  setNumberOfRows(_data[0].size());
  setNumberOfCols(_data.size());
}

Real
CSVSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  // Checks to make sure that the row and column indices are not out of bounds
  mooseAssert(row_index < _data[0].size(), "row_index cannot be out of bounds of the data.");
  mooseAssert(col_index < _data.size(), "col_index cannot be out of bounds of the data.");

  return _data[col_index][row_index]; // entering samples into the matrix
}
