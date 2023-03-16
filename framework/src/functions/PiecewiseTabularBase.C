//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseTabularBase.h"
#include "DelimitedFileReader.h"
#include "libmesh/int_range.h"

InputParameters
PiecewiseTabularBase::validParams()
{
  InputParameters params = PiecewiseBase::validParams();

  MooseEnum axis("x=0 y=1 z=2");
  params.addParam<MooseEnum>(
      "axis", axis, "The axis used (x, y, or z) if this is to be a function of position");
  params.addParam<std::vector<Real>>("xy_data",
                                     "All function data, supplied in abscissa, ordinate pairs");
  params.addParam<std::vector<Real>>("x", "The abscissa values");
  params.addParam<std::vector<Real>>("y", "The ordinate values");
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
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  params.declareControllable("scale_factor");
  return params;
}

PiecewiseTabularBase::PiecewiseTabularBase(const InputParameters & parameters)
  : PiecewiseBase(parameters),
    _scale_factor(this->template getParam<Real>("scale_factor")),
    _has_axis(isParamValid("axis"))
{
  // determine function argument
  if (_has_axis)
    _axis = this->template getParam<MooseEnum>("axis");

  // determine data source and check parameter consistency
  if (isParamValid("data_file") && !isParamValid("x") && !isParamValid("y") &&
      !isParamValid("xy_data"))
    buildFromFile();
  else if (!isParamValid("data_file") && isParamValid("x") && isParamValid("y") &&
           !isParamValid("xy_data"))
    buildFromXandY();
  else if (!isParamValid("data_file") && !isParamValid("x") && !isParamValid("y") &&
           isParamValid("xy_data"))
    buildFromXY();
  else
    mooseError("In ",
               _name,
               ": Either 'data_file', 'x' and 'y', or 'xy_data' must be specified exclusively.");
}

void
PiecewiseTabularBase::buildFromFile()
{
  // Input parameters
  const auto & data_file_name = this->template getParam<FileName>("data_file");
  const MooseEnum format = this->template getParam<MooseEnum>("format");

  // xy_in_file_only does not make sense here as it restricts the file to exactly
  // two cows/columns, which is not a likely scenario when looking up data by name.
  // A wrong 'format' parameter would be caught by the failing name resolution anyways.
  bool xy_only = this->template getParam<bool>("xy_in_file_only");
  if (isParamValid("x_title") || isParamValid("y_title"))
  {
    if (this->_pars.isParamSetByUser("xy_in_file_only") && xy_only)
      paramError(
          "xy_in_file_only",
          "When accessing data through 'x_title' or 'y_title' this parameter should not be used");
    else
      xy_only = false;
  }

  // Read the data from CSV file
  MooseUtils::DelimitedFileReader reader(data_file_name, &_communicator);
  reader.setFormatFlag(format.getEnum<MooseUtils::DelimitedFileReader::FormatFlag>());
  reader.setComment("#");
  reader.read();

  const auto & columns = reader.getNames();
  const auto & data = reader.getData();

  if (data.size() > 2 && xy_only)
    mooseError("In ",
               _name,
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
    if (isParamValid(prefix + "_title"))
    {
      const auto name = this->template getParam<std::string>(prefix + "_title");
      for (const auto i : index_range(columns))
        if (columns[i] == name)
          index = i;

      if (index == libMesh::invalid_uint)
        paramError(prefix + "_title",
                   "None of the ",
                   format,
                   " in the data file has the title '",
                   name,
                   "'.");
    }
    else
      index = this->template getParam<unsigned int>(prefix + "_index_in_file");

    if (index >= data.size())
      paramError(prefix + "_index_in_file",
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
    mooseError(
        "In ", _name, ": 'x_index_in_file' and 'y_index_in_file' are set to the same value.");

  // Update the input vectors to contained the desired data
  _raw_x = reader.getData(x_index);
  _raw_y = reader.getData(y_index);

  // Size mismatch error
  if (_raw_x.size() != _raw_y.size())
    mooseError("In ", _name, ": Lengths of x and y data do not match.");
}

void
PiecewiseTabularBase::buildFromXandY()
{
  _raw_x = this->template getParam<std::vector<Real>>("x");
  _raw_y = this->template getParam<std::vector<Real>>("y");
}

void
PiecewiseTabularBase::buildFromXY()
{
  const auto & xy = this->template getParam<std::vector<Real>>("xy_data");
  const auto xy_size = xy.size();
  if (xy_size % 2 != 0)
    mooseError("In ", _name, ": Length of data provided in 'xy_data' must be a multiple of 2.");

  const auto data_size = xy_size / 2;
  _raw_x.resize(data_size);
  _raw_y.resize(data_size);
  for (const auto i : make_range(data_size))
  {
    _raw_x[i] = xy[2 * i];
    _raw_y[i] = xy[2 * i + 1];
  }
}
