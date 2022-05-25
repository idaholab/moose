//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationData.h"
#include "DelimitedFileReader.h"

registerMooseObject("isopodApp", OptimizationData);

InputParameters
OptimizationData::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Reporter to hold measurement and simulation data for optimization problems");

  params.addParam<std::vector<Real>>(
      "measurement_values",
      "Measurement values collected from locations given by measurement_points");
  params.addParam<std::vector<Point>>("measurement_points",
                                      "Point locations corresponding to each measurement value");

  params.addParam<FileName>("measurement_file",
                            "CSV file with measurement value and coordinates (value, x, y, z).");
  params.addParam<std::string>(
      "file_xcoord", "x", "x coordinate column name from measurement_file csv being read in.");
  params.addParam<std::string>(
      "file_ycoord", "y", "y coordinate column name from csv file being read in");
  params.addParam<std::string>(
      "file_zcoord", "z", "z coordinate column name from csv file being read in");
  params.addParam<std::string>(
      "file_value", "value", "measurement value column name from csv file being read in");
  return params;
}

OptimizationData::OptimizationData(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _measurement_xcoord(
        declareValueByName<std::vector<Real>>("measurement_xcoord", REPORTER_MODE_REPLICATED)),
    _measurement_ycoord(
        declareValueByName<std::vector<Real>>("measurement_ycoord", REPORTER_MODE_REPLICATED)),
    _measurement_zcoord(
        declareValueByName<std::vector<Real>>("measurement_zcoord", REPORTER_MODE_REPLICATED)),
    _measurement_values(
        declareValueByName<std::vector<Real>>("measurement_values", REPORTER_MODE_REPLICATED)),
    _simulation_values(
        declareValueByName<std::vector<Real>>("simulation_values", REPORTER_MODE_REPLICATED)),
    _misfit_values(declareValueByName<std::vector<Real>>("misfit_values", REPORTER_MODE_REPLICATED))
{
  if (isParamValid("measurement_file"))
    readMeasurementsFromFile();
  else if (isParamValid("measurement_points"))
    readMeasurementsFromInput();
}

void
OptimizationData::execute()
{
}

void
OptimizationData::readMeasurementsFromFile()
{
  std::string xName = getParam<std::string>("file_xcoord");
  std::string yName = getParam<std::string>("file_ycoord");
  std::string zName = getParam<std::string>("file_zcoord");
  std::string valueName = getParam<std::string>("file_value");

  bool found_x = false;
  bool found_y = false;
  bool found_z = false;
  bool found_value = false;

  MooseUtils::DelimitedFileReader reader(getParam<FileName>("measurement_file"));
  reader.read();

  auto const & names = reader.getNames();
  auto const & data = reader.getData();

  const std::size_t rows = data[0].size();
  for (std::size_t i = 0; i < names.size(); ++i)
  {
    // make sure all data columns have the same length
    if (data[i].size() != rows)
      paramError("file", "Mismatching column lengths in file");

    if (names[i] == xName)
    {
      for (auto && d : data[i])
        _measurement_xcoord.push_back(d);
      found_x = true;
    }
    else if (names[i] == yName)
    {
      for (auto && d : data[i])
        _measurement_ycoord.push_back(d);
      found_y = true;
    }
    else if (names[i] == zName)
    {
      for (auto && d : data[i])
        _measurement_zcoord.push_back(d);
      found_z = true;
    }
    else if (names[i] == valueName)
    {
      for (auto && d : data[i])
        _measurement_values.push_back(d);
      found_value = true;
    }
  }

  // check if all required columns were found
  if (!found_x)
    paramError("measurement_file", "Column with name '", xName, "' missing from measurement file");
  else if (!found_y)
    paramError("measurement_file", "Column with name '", yName, "' missing from measurement file");
  else if (!found_z)
    paramError("measurement_file", "Column with name '", zName, "' missing from measurement file");
  else if (!found_value)
    paramError(
        "measurement_file", "Column with name '", valueName, "' missing from measurement file");
}

void
OptimizationData::readMeasurementsFromInput()
{
  std::vector<Point> measurement_points = getParam<std::vector<Point>>("measurement_points");
  for (auto & p : measurement_points)
  {
    _measurement_xcoord.push_back(p(0));
    _measurement_ycoord.push_back(p(1));
    _measurement_zcoord.push_back(p(2));
  }

  if (isParamValid("measurement_values"))
    _measurement_values = getParam<std::vector<Real>>("measurement_values");
  else
    paramError("measurement_values", "Input file must contain measurement points and values");
}
