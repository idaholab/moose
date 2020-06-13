//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseBase.h"
#include "DelimitedFileReader.h"

defineLegacyParams(PiecewiseBase);

InputParameters
PiecewiseBase::validParams()
{
  InputParameters params = Function::validParams();

  MooseEnum axis("x=0 y=1 z=2");
  params.addParam<MooseEnum>(
      "axis", axis, "The axis used (x, y, or z) if this is to be a function of position");
  params.addParam<std::vector<Real>>("xy_data",
                                     "All function data, supplied in abscissa, ordinate pairs");
  params.addParam<std::vector<Real>>("x", "The abscissa values");
  params.addParam<std::vector<Real>>("y", "The ordinate values");
  params.addParam<FileName>("data_file", "File holding CSV data for use with Piecewise");
  params.addParam<unsigned int>("x_index_in_file", 0, "The abscissa index in the data file");
  params.addParam<unsigned int>("y_index_in_file", 1, "The ordinate index in the data file");
  params.addParam<bool>(
      "xy_in_file_only", true, "If the data file only contains abscissa and ordinate data");

  MooseEnum format("columns=0 rows=1", "rows");
  params.addParam<MooseEnum>(
      "format", format, "Format of csv data file that is in either in columns or rows");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  params.declareControllable("scale_factor");
  return params;
}

PiecewiseBase::PiecewiseBase(const InputParameters & parameters)
  : Function(parameters),
    _scale_factor(getParam<Real>("scale_factor")),
    _has_axis(isParamValid("axis"))
{
  // determine function argument
  if (_has_axis)
    _axis = getParam<MooseEnum>("axis");

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
    mooseError("In Piecewise ",
               _name,
               ": Either 'data_file', 'x' and 'y', or 'xy_data' must be specified exclusively.");

  // Size mismatch error
  if (_raw_x.size() != _raw_y.size())
    mooseError("In PiecewiseBase ", _name, ": Lengths of x and y data do not match.");
}

Real
PiecewiseBase::functionSize() const
{
  return _raw_x.size();
}

Real
PiecewiseBase::domain(const int i) const
{
  return _raw_x[i];
}

Real
PiecewiseBase::range(const int i) const
{
  return _raw_y[i];
}

void
PiecewiseBase::buildFromFile()
{
  // Input parameters
  const FileName & data_file_name = getParam<FileName>("data_file");
  const MooseEnum & format = getParam<MooseEnum>("format");
  unsigned int x_index = getParam<unsigned int>("x_index_in_file");
  unsigned int y_index = getParam<unsigned int>("y_index_in_file");
  bool xy_only = getParam<bool>("xy_in_file_only");

  if (x_index == y_index)
    mooseError("In Piecewise ",
               _name,
               ": 'x_index_in_file' and 'y_index_in_file' are set to the same value.");

  // Read the data from CSV file
  MooseUtils::DelimitedFileReader reader(data_file_name, &_communicator);
  reader.setFormatFlag(format.getEnum<MooseUtils::DelimitedFileReader::FormatFlag>());
  reader.setComment("#");
  reader.read();
  const std::vector<std::vector<double>> & data = reader.getData();

  // Check the data format
  if (x_index >= data.size())
    mooseError("In Piecewise ",
               _name,
               ": The 'x_index_in_file' is out-of-range of the available data in '",
               data_file_name,
               "', which contains ",
               data.size(),
               " ",
               format,
               " of data.");

  if (y_index >= data.size())
    mooseError("In Piecewise ",
               _name,
               ": The 'y_index_in_file' is out-of-range of the available data in '",
               data_file_name,
               "', which contains ",
               data.size(),
               " ",
               format,
               " of data.");

  if (data.size() > 2 && xy_only)
    mooseError("In Piecewise ",
               _name,
               ": Read more than two ",
               format,
               " of data from file '",
               data_file_name,
               "'.  Did you mean to use \"format = ",
               format == "columns" ? "rows" : "columns",
               "\" or set \"xy_in_file_only\" to false?");

  // Update the input vectors to contained the desired data
  _raw_x = reader.getData(x_index);
  _raw_y = reader.getData(y_index);
}

void
PiecewiseBase::setData(const std::vector<Real> & x, const std::vector<Real> & y)
{
  _raw_x = x;
  _raw_y = y;
  if (_raw_x.size() != _raw_y.size())
    mooseError("In PiecewiseBase ", _name, ": Lengths of x and y data do not match.");
}

void
PiecewiseBase::buildFromXandY()
{
  _raw_x = getParam<std::vector<Real>>("x");
  _raw_y = getParam<std::vector<Real>>("y");
}

void
PiecewiseBase::buildFromXY()
{
  std::vector<Real> xy = getParam<std::vector<Real>>("xy_data");
  unsigned int xy_size = xy.size();
  if (xy_size % 2 != 0)
    mooseError(
        "In Piecewise ", _name, ": Length of data provided in 'xy_data' must be a multiple of 2.");

  unsigned int data_size = xy_size / 2;
  _raw_x.reserve(data_size);
  _raw_y.reserve(data_size);
  for (unsigned int i = 0; i < xy_size; i += 2)
  {
    _raw_x.push_back(xy[i]);
    _raw_y.push_back(xy[i + 1]);
  }
}
