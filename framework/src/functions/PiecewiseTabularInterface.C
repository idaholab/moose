//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseTabularInterface.h"
#include "DelimitedFileReader.h"
#include "JSONFileReader.h"

#include "libmesh/int_range.h"

InputParameters
PiecewiseTabularInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  // Parameters shared across all data input methods
  MooseEnum axis("x=0 y=1 z=2");
  params.addParam<MooseEnum>(
      "axis", axis, "The axis used (x, y, or z) if this is to be a function of position");

  // Data from input file parameters
  params.addParam<std::vector<Real>>("xy_data",
                                     "All function data, supplied in abscissa, ordinate pairs");
  params.addParam<std::vector<Real>>("x", "The abscissa values");
  params.addParam<std::vector<Real>>("y", "The ordinate values");

  // Data from CSV file parameters
  params.addParam<FileName>("data_file", "File holding CSV data");
  params.addParam<unsigned int>("x_index_in_file", 0, "The abscissa index in the data file");
  params.addParam<unsigned int>("y_index_in_file", 1, "The ordinate index in the data file");
  params.addParam<std::string>(
      "x_title", "The title of the column/row containing the x data in the data file");
  params.addParam<std::string>(
      "y_title", "The title of the column/row containing the y data in the data file");
  params.addParam<bool>(
      "xy_in_file_only", true, "If the data file only contains abscissa and ordinate data");
  MooseEnum format("columns=0 rows=1", "rows");
  params.addParam<MooseEnum>(
      "format", format, "Format of csv data file that is in either in columns or rows");

  // Data from JSON parameters
  params.addParam<UserObjectName>("json_uo", "JSONFileReader holding the data");
  params.addParam<std::vector<std::string>>(
      "x_keys", "Ordered vector of keys in the JSON tree to obtain the abscissa");
  params.addParam<std::vector<std::string>>(
      "y_keys", "Ordered vector of keys in the JSON tree to obtain the ordinate");

  params.addParamNamesToGroup("xy_data x y", "Data from input file");
  params.addParamNamesToGroup(
      "data_file x_index_in_file y_index_in_file x_title y_title xy_in_file_only format",
      "Data from CSV file");
  params.addParamNamesToGroup("json_uo x_keys y_keys", "Data from JSON");

  return params;
}

PiecewiseTabularInterface::PiecewiseTabularInterface(const MooseObject & object,
                                                     std::vector<Real> & data_x,
                                                     std::vector<Real> & data_y)
  : _has_axis(object.isParamValid("axis")),
    _object(object),
    _parameters(object.parameters()),
    _data_x(data_x),
    _data_y(data_y)
{
  // determine function argument
  if (_has_axis)
    _axis = _parameters.get<MooseEnum>("axis");

  // Check all parameters for inconsistencies
  if (_parameters.isParamValid("data_file") + _parameters.isParamValid("json_uo") +
          (_parameters.isParamValid("x") || _parameters.isParamValid("y")) +
          _parameters.isParamValid("xy_data") >
      1)
    _object.mooseError(
        "Either 'data_file' or 'json_uo' or 'x' and 'y' or 'xy_data' must be specified "
        "exclusively.");
  if ((_parameters.isParamValid("x") + _parameters.isParamValid("y")) % 2 != 0)
    _object.mooseError(
        "Both 'x' and 'y' parameters or neither (for another input method) must be specified");
  if (!_parameters.isParamValid("data_file") &&
      (_parameters.isParamSetByUser("x_index_in_file") ||
       _parameters.isParamSetByUser("y_index_in_file") || _parameters.isParamValid("x_title") ||
       _parameters.isParamValid("y_title") || _parameters.isParamSetByUser("xy_in_file_only") ||
       _parameters.isParamSetByUser("format")))
    _object.mooseError(
        "A parameter was passed for an option using data from a CSV file but the "
        "'data_file' parameter has not been set. This is not allowed. Please check the parameter "
        "groups in the documentation for the list of parameters for each data input method.");
  if (!_parameters.isParamValid("json_uo") &&
      (_parameters.isParamValid("x_keys") || _parameters.isParamValid("y_keys")))
    _object.mooseError(
        "A parameter was passed for a JSON input option but the 'json_uo' parameter has not "
        "been set. This is not allowed. Please check the parameter groups in the "
        "documentation for the list of parameters for each data input method.");
}

void
PiecewiseTabularInterface::buildFromFile(const libMesh::Parallel::Communicator & comm)
{
  // Input parameters
  const auto & data_file_name = _parameters.get<FileName>("data_file");
  const MooseEnum format = _parameters.get<MooseEnum>("format");

  // xy_in_file_only does not make sense here as it restricts the file to exactly
  // two cows/columns, which is not a likely scenario when looking up data by name.
  // A wrong 'format' parameter would be caught by the failing name resolution anyways.
  bool xy_only = _parameters.get<bool>("xy_in_file_only");
  if (_parameters.isParamValid("x_title") || _parameters.isParamValid("y_title"))
  {
    if (_parameters.isParamSetByUser("xy_in_file_only") && xy_only)
      _parameters.paramError(
          "xy_in_file_only",
          "When accessing data through 'x_title' or 'y_title' this parameter should not be used");
    else
      xy_only = false;
  }

  // Read the data from CSV file
  MooseUtils::DelimitedFileReader reader(data_file_name, &comm);
  reader.setFormatFlag(format.getEnum<MooseUtils::DelimitedFileReader::FormatFlag>());
  reader.setComment("#");
  reader.read();

  const auto & columns = reader.getNames();
  const auto & data = reader.getData();

  if (data.size() > 2 && xy_only)
    _object.mooseError("In ",
                       _object.name(),
                       ": Read more than two ",
                       format,
                       " of data from file '",
                       data_file_name,
                       "'.  Did you mean to use \"format = ",
                       format == "columns" ? "rows" : "columns",
                       "\" or set \"xy_in_file_only\" to false?");

  unsigned int x_index = libMesh::invalid_uint;
  unsigned int y_index = libMesh::invalid_uint;
  const auto setIndex = [&](unsigned int & index, const std::string & prefix)
  {
    if (_parameters.isParamValid(prefix + "_title"))
    {
      const auto name = _parameters.get<std::string>(prefix + "_title");
      for (const auto i : index_range(columns))
        if (columns[i] == name)
          index = i;

      if (index == libMesh::invalid_uint)
        _parameters.paramError(prefix + "_title",
                               "None of the ",
                               format,
                               " in the data file has the title '",
                               name,
                               "'.");
    }
    else
      index = _parameters.get<unsigned int>(prefix + "_index_in_file");

    if (index >= data.size())
      _parameters.paramError(prefix + "_index_in_file",
                             "Index out-of-range of the available data in '",
                             data_file_name,
                             "', which contains ",
                             data.size(),
                             " ",
                             format,
                             " of data.");
  };

  setIndex(x_index, "x");
  setIndex(y_index, "y");

  if (x_index == y_index)
    _object.mooseError("In ",
                       _object.name(),
                       ": 'x_index_in_file' and 'y_index_in_file' are set to the same value.");

  // Update the input vectors to contained the desired data
  _data_x = reader.getData(x_index);
  _data_y = reader.getData(y_index);

  // Size mismatch error
  if (_data_x.size() != _data_y.size())
    _object.mooseError("In ", _object.name(), ": Lengths of x and y data do not match.");

  _raw_data_loaded = true;
}

void
PiecewiseTabularInterface::buildFromJSON(const JSONFileReader & json_uo)
{
  if (!_parameters.isParamValid("x_keys"))
    _object.mooseError("Missing 'x_keys' parameters for loading data from JSON");
  if (!_parameters.isParamValid("y_keys"))
    _object.mooseError("Missing 'y_keys' parameters for loading data from JSON");
  json_uo.getVector<Real>(_parameters.get<std::vector<std::string>>("x_keys"), _data_x);
  json_uo.getVector<Real>(_parameters.get<std::vector<std::string>>("y_keys"), _data_y);
  _raw_data_loaded = true;

  // Size mismatch error
  if (_data_x.size() != _data_y.size())
    _object.mooseError(
        "Lengths of x (", _data_x.size(), ") and y (", _data_y.size(), ") data do not match.");
}

void
PiecewiseTabularInterface::buildFromXandY()
{
  _data_x = _parameters.get<std::vector<Real>>("x");
  _data_y = _parameters.get<std::vector<Real>>("y");
  _raw_data_loaded = true;
}

void
PiecewiseTabularInterface::buildFromXY()
{
  const auto & xy = _parameters.get<std::vector<Real>>("xy_data");
  const auto xy_size = xy.size();
  if (xy_size % 2 != 0)
    _object.mooseError(
        "In ", _object.name(), ": Length of data provided in 'xy_data' must be a multiple of 2.");

  const auto data_size = xy_size / 2;
  _data_x.resize(data_size);
  _data_y.resize(data_size);
  for (const auto i : make_range(data_size))
  {
    _data_x[i] = xy[2 * i];
    _data_y[i] = xy[2 * i + 1];
  }
  _raw_data_loaded = true;
}
