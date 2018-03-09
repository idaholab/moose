//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DelimitedFileReader.h"
#include <glob.h>
#include "CSVInterpolator.h"
#include "BilinearInterpolation.h"

template <>
InputParameters
validParams<CSVInterpolator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription(
      "Reads a given position column and variable data from set of "
      "CSV files and creates a bilinear interpolation "
      "object in position and time. Note that each CSV file contains output from one time step.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONE;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.addRequiredParam<std::string>("csv_filename_pattern",
                                       "The filename pattern (glob) for the csv files.");
  params.addRequiredParam<unsigned int>(
      "from_component",
      "Component that need to extracted from the csv FileMesh. x = 0, y = 1 and z = 2.");
  params.addRequiredParam<std::vector<std::string>>(
      "variable_vectors", "Names of variable vectors that need to be extracted from csv files.");
  params.addParam<Real>("time_step",
                        "The constant time step at which the csv file output is provided.");
  params.addParam<FileName>("time_file",
                            "The file that contains the time information if a "
                            "constant time step is not used to generate "
                            "CSV output. The column corresponding to "
                            "time is obtained using the 'time' header flag.");
  return params;
}

CSVInterpolator::CSVInterpolator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _pattern(getParam<std::string>("csv_filename_pattern")),
    _from_component(getParam<unsigned int>("from_component")),
    _variable_vectors(getParam<std::vector<std::string>>("variable_vectors")),
    _bilinear_interp(_variable_vectors.size())
{
  if (_variable_vectors.size() == 0)
    mooseError("CSVInterpolator: Please provide a non-empty list of variable vectors.");

  // Get all file names matching the given pattern
  std::vector<std::string> file_names = glob(_pattern);
  if (file_names.empty())
    mooseError("Unable to locate files with the given pattern (",
               _pattern,
               ") in the CSVInterpolator object '",
               name(),
               "'.");

  std::vector<std::unique_ptr<MooseUtils::DelimitedFileReader>> readers;
  readers.reserve(file_names.size());

  // Create time, position and value vectors
  std::vector<Real> t(file_names.size(), 0.0);
  std::vector<Real> position;
  ColumnMajorMatrix A;
  std::vector<ColumnMajorMatrix> value(_variable_vectors.size(), A);

  if (isParamValid("time_step") && !isParamValid("time_file"))
  {
    Real dt = getParam<Real>("time_step");
    for (unsigned int i = 0; i < file_names.size(); ++i)
      t[i] = i * dt;
  }
  else if (!isParamValid("time_step") && isParamValid("time_file"))
  {
    std::string time_file = getParam<std::string>("time_file");
    MooseUtils::DelimitedFileReader file = MooseUtils::DelimitedFileReader(time_file);
    file.read();
    t = file.getData("time");
    if (t.size() != file_names.size())
      mooseError("CSVInterpolator: The number of CSV files and "
                 "the length of the time vector should be same.");
  }
  else if (isParamValid("time_step") && isParamValid("time_file"))
    mooseError("CSVInterpolator: Please provide either time step or time file "
               "not both.");
  else
    mooseError("CSVInterpolator: Either time step or time file should be provided.");

  // First CSV file does not usually contain any data
  if (file_names.size() > 1)
  {
    for (unsigned int i = 1; i < file_names.size(); ++i)
    {
      std::string filename = file_names[i];
      MooseUtils::DelimitedFileReader reader = MooseUtils::DelimitedFileReader(filename);
      reader.read();

      // Read position data from the first file with data
      if (i == 1)
      {
        if (_from_component == 0)
          position = reader.getData("x");
        else if (_from_component == 1)
          position = reader.getData("y");
        else if (_from_component == 2)
          position = reader.getData("z");
        else
          mooseError("CSVInterpolator: from_component should be 0, 1 or 2.");

        // Resize the ColumnMajorMatrix corresponding to each variable value
        for (unsigned int j = 0; j < _variable_vectors.size(); ++j)
        {
          value[j].reshape(file_names.size(), position.size());
          value[j].zero();
        }
      }

      // Stored variable values in the ColumnMajorMatrix
      for (unsigned int j = 0; j < _variable_vectors.size(); ++j)
      {
        std::vector<Real> temp = reader.getData(_variable_vectors[j]);
        for (unsigned int k = 0; k < temp.size(); ++k)
          value[j](i, k) = temp[k];
      }
    }
  }
  for (unsigned int i = 0; i < _variable_vectors.size(); ++i)
    _bilinear_interp[i] = libmesh_make_unique<BilinearInterpolation>(position, t, value[i]);
}

std::vector<Real>
CSVInterpolator::getValue(const Real & x, const Real & y) const
{
  std::vector<Real> value(_variable_vectors.size());
  for (unsigned int i = 0; i < _variable_vectors.size(); ++i)
    value[i] = _bilinear_interp[i]->sample(x, y);

  return value;
}

std::vector<std::string>
CSVInterpolator::glob(const std::string & pattern)
{
  glob_t glob_result;
  ::glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
  std::vector<std::string> output;
  for (unsigned int i = 0; i < glob_result.gl_pathc; ++i)
    output.push_back(std::string(glob_result.gl_pathv[i]));
  globfree(&glob_result);
  return output;
}
