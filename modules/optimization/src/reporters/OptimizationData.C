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
#include "SystemBase.h"

registerMooseObject("OptimizationApp", OptimizationData);

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
  params.addParam<std::vector<Real>>("measurement_times",
                                     "Times corresponding to each measurement value");

  params.addParam<FileName>("measurement_file",
                            "CSV file with measurement value and coordinates (value, x, y, z).");
  params.addParam<std::string>(
      "file_xcoord", "x", "x coordinate column name from measurement_file csv being read in.");
  params.addParam<std::string>(
      "file_ycoord", "y", "y coordinate column name from csv file being read in.");
  params.addParam<std::string>(
      "file_zcoord", "z", "z coordinate column name from csv file being read in.");
  params.addParam<std::string>(
      "file_time", "time", "time column name from csv file being read in.");
  params.addParam<std::vector<std::string>>(
      "file_variable_weights", "variable weight column names from csv file being read in.");
  params.addParam<std::string>(
      "file_value", "value", "measurement value column name from csv file being read in.");

  params.addParam<std::vector<std::string>>(
      "variable_weight_names",
      "Vector of weight reporter names that will create a reporter to transfer weights into.  The "
      "ordering of these weight reporter names corresponds to the ordering used in variable.");
  params.addParam<std::vector<VariableName>>(
      "variable", "Vector of variable names to sample at measurement points.");

  params.addParamNamesToGroup("measurement_points measurement_values measurement_times",
                              "Input Measurement Data");
  params.addParamNamesToGroup("measurement_file file_xcoord file_ycoord file_zcoord file_time "
                              "file_value file_variable_weights",
                              "File Measurement Data");
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
    _measurement_time(
        declareValueByName<std::vector<Real>>("measurement_time", REPORTER_MODE_REPLICATED)),
    _measurement_values(
        declareValueByName<std::vector<Real>>("measurement_values", REPORTER_MODE_REPLICATED)),
    _simulation_values(
        declareValueByName<std::vector<Real>>("simulation_values", REPORTER_MODE_REPLICATED)),
    _misfit_values(declareValueByName<std::vector<Real>>("misfit_values", REPORTER_MODE_REPLICATED))
{
  // read in data
  if (isParamValid("measurement_file") && isParamValid("measurement_points"))
    mooseError("Input file can only define a single input for measurement data. Use only "
               "measurement_file or measurement_points, but never both");
  else if (isParamValid("measurement_file"))
    readMeasurementsFromFile();
  else if (isParamValid("measurement_points"))
    readMeasurementsFromInput();

  _misfit_values.resize(_measurement_values.size());

  if (isParamValid("variable"))
  {
    std::vector<VariableName> var_names(getParam<std::vector<VariableName>>("variable"));
    for (const auto & name : var_names)
      _var_vec.push_back(&_fe_problem.getVariable(
          _tid, name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD));
  }
  if (isParamValid("variable_weight_names"))
  {
    std::vector<std::string> weight_names(
        getParam<std::vector<std::string>>("variable_weight_names"));
    for (const auto & name : weight_names)
    {
      if (_weight_names_weights_map.count(name) == 1)
      {
        _variable_weights.push_back(_weight_names_weights_map[name]);
      }
      else
      {
        // default is to create a new weight reporter and fill it with 1's
        // these will be overwritten by a reporter transfer.
        _variable_weights.push_back(
            &declareValueByName<std::vector<Real>>(name, REPORTER_MODE_REPLICATED));
        _variable_weights.back()->assign(_measurement_xcoord.size(), 1);
      }
    }
  }
  if (isParamValid("variable") && isParamValid("variable_weight_names") &&
      _variable_weights.size() != _var_vec.size())
  {
    paramError("variable_weight_names",
               "The same number of names must be in both 'variable_weight_names' and 'variable'.");
  }
}

void
OptimizationData::execute()
{
  if (_var_vec.empty())
    return;

  // FIXME: This is basically copied from PointValue.
  // Implementation can be improved using the functionality in PointSamplerBase,
  // but this will require changes in MOOSE to work for reporters.

  const std::size_t nvals = _measurement_values.size();
  _simulation_values.resize(nvals, 0.0);
  _misfit_values.resize(nvals);

  errorCheckDataSize();
  for (const auto var_index : make_range(_var_vec.size()))
  {
    const auto & sys = _var_vec[var_index]->sys().system();
    const auto vnum = _var_vec[var_index]->number();
    // A weight reporter is not automatically created and for those cases, we
    // set the weight to 1.
    std::vector<Real> weights(_variable_weights.empty()
                                  ? std::vector<Real>(_measurement_xcoord.size(), 1)
                                  : (*_variable_weights[var_index]));
    for (const auto & i : make_range(nvals))
    {
      if (MooseUtils::absoluteFuzzyEqual(_t, _measurement_time[i]))
      {
        const Point point(_measurement_xcoord[i], _measurement_ycoord[i], _measurement_zcoord[i]);
        const Real val = sys.point_value(vnum, point, false);

        _simulation_values[i] += weights[i] * val;
        _misfit_values[i] = _simulation_values[i] - _measurement_values[i];
      }
    }
  }
}

void
OptimizationData::readMeasurementsFromFile()
{
  std::string xName = getParam<std::string>("file_xcoord");
  std::string yName = getParam<std::string>("file_ycoord");
  std::string zName = getParam<std::string>("file_zcoord");
  std::string tName = getParam<std::string>("file_time");
  std::string valueName = getParam<std::string>("file_value");
  std::vector<std::string> weightNames =
      getParam<std::vector<std::string>>("file_variable_weights");

  bool found_x = false;
  bool found_y = false;
  bool found_z = false;
  bool found_t = false;
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
      _measurement_xcoord = data[i];
      found_x = true;
    }
    else if (names[i] == yName)
    {
      _measurement_ycoord = data[i];
      found_y = true;
    }
    else if (names[i] == zName)
    {
      _measurement_zcoord = data[i];
      found_z = true;
    }
    else if (names[i] == tName)
    {
      _measurement_time = data[i];
      found_t = true;
    }
    else if (names[i] == valueName)
    {
      _measurement_values = data[i];
      found_value = true;
    }
    else if (std::find(weightNames.begin(), weightNames.end(), names[i]) != weightNames.end())
    {
      _weight_names_weights_map.emplace(
          names[i], &(declareValueByName<std::vector<Real>>(names[i], REPORTER_MODE_REPLICATED)));
      _weight_names_weights_map[names[i]]->assign(data[i].begin(), data[i].end());
    }
  }

  // check if all required columns were found
  if (!found_x)
    paramError("measurement_file", "Column with name '", xName, "' missing from measurement file");
  if (!found_y)
    paramError("measurement_file", "Column with name '", yName, "' missing from measurement file");
  if (!found_z)
    paramError("measurement_file", "Column with name '", zName, "' missing from measurement file");
  if (!found_t)
    _measurement_time.assign(rows, 0);
  if (!found_value)
    paramError(
        "measurement_file", "Column with name '", valueName, "' missing from measurement file");
  if (_weight_names_weights_map.size() != weightNames.size())
  {
    std::string out("\n   Measurement file column names: ");
    for (const auto & name : names)
      out += " " + name;
    out += "\n   file_variable_weights names: ";
    for (const auto & name : weightNames)
      out += " " + name;
    paramError("measurement_file",
               "Not all of the file_variable_weights names were found in the measurement_file.",
               out);
  }
}

void
OptimizationData::readMeasurementsFromInput()
{
  if (isParamValid("file_variable_weights"))
    paramError("measurement_values",
               "file_variable_weights cannot be used with measurement data read from the input "
               "file, use measure_file input instead.");

  for (const auto & p : getParam<std::vector<Point>>("measurement_points"))
  {
    _measurement_xcoord.push_back(p(0));
    _measurement_ycoord.push_back(p(1));
    _measurement_zcoord.push_back(p(2));
  }

  if (isParamValid("measurement_times"))
    _measurement_time = getParam<std::vector<Real>>("measurement_times");
  else
    _measurement_time.assign(_measurement_xcoord.size(), 0.0);

  if (isParamValid("measurement_values"))
    _measurement_values = getParam<std::vector<Real>>("measurement_values");
  else
    paramError("measurement_values", "Input file must contain measurement points and values");
}

void
OptimizationData::errorCheckDataSize()
{
  const std::size_t nvals = _measurement_values.size();
  std::string msg = "";
  if (_measurement_xcoord.size() != nvals)
    msg += "x-coordinate data (" + std::to_string(_measurement_xcoord.size()) + "), ";
  if (_measurement_ycoord.size() != nvals)
    msg += "y-coordinate data (" + std::to_string(_measurement_ycoord.size()) + "), ";
  if (_measurement_zcoord.size() != nvals)
    msg += "z-coordinate data (" + std::to_string(_measurement_zcoord.size()) + "), ";
  if (_measurement_time.size() != nvals)
    msg += "time data (" + std::to_string(_measurement_time.size()) + "), ";
  if (!msg.empty())
    mooseError("Number of entries in ",
               std::string(msg.begin(), msg.end() - 2),
               " does not match number of entries in value data (",
               std::to_string(nvals),
               ").");
}
