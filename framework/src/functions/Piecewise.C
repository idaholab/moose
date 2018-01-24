//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Piecewise.h"
#include "DelimitedFileReader.h"

#include <fstream>

template <>
InputParameters
validParams<Piecewise>()
{
  InputParameters params = validParams<Function>();
  params.addParam<std::vector<Real>>("xy_data",
                                     "All function data, supplied in abscissa, ordinate pairs");
  params.addParam<std::vector<Real>>("x", "The abscissa values");
  params.addParam<std::vector<Real>>("y", "The ordinate values");
  params.addParam<FileName>("data_file", "File holding csv data for use with Piecewise");
  params.addParam<unsigned int>("x_index_in_file", 0, "The abscissa index in the data file");
  params.addParam<unsigned int>("y_index_in_file", 1, "The ordinate index in the data file");
  params.addParam<bool>(
      "xy_in_file_only", true, "If the data file only contains abscissa and ordinate data");

  MooseEnum format("columns=0 rows=1", "rows");
  params.addParam<MooseEnum>(
      "format", format, "Format of csv data file that is in either in columns or rows");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");

  MooseEnum axis("x=0 y=1 z=2 0=3 1=4 2=5");
  axis.deprecate("0", "x");
  axis.deprecate("1", "y");
  axis.deprecate("2", "z");
  params.addParam<MooseEnum>(
      "axis", axis, "The axis used (x, y, or z) if this is to be a function of position");
  return params;
}

Piecewise::Piecewise(const InputParameters & parameters)
  : Function(parameters), _scale_factor(getParam<Real>("scale_factor")), _has_axis(false)
{
  std::pair<std::vector<Real>, std::vector<Real>> xy;

  if (isParamValid("data_file"))
    xy = buildFromFile();

  else if (isParamValid("x") || isParamValid("y"))
    xy = buildFromXandY();

  else if (isParamValid("xy_data"))
    xy = buildFromXY();

  else
    mooseError("In Piecewise ",
               _name,
               ": Either 'data_file', 'x' and 'y', or 'xy_data' must be specified.");

  setData(xy.first, xy.second);
}

void
Piecewise::setData(const std::vector<Real> & x, const std::vector<Real> & y)
{
  // Size mismatch error
  if (x.size() != y.size())
    mooseError("In Piecewise ", _name, ": Lengths of x and y data do not match.");

  try
  {
    _linear_interp = libmesh_make_unique<LinearInterpolation>(x, y);
  }
  catch (std::domain_error & e)
  {
    mooseError("In Piecewise ", _name, ": ", e.what());
  }

  if (isParamValid("axis"))
  {
    const MooseEnum & axis = getParam<MooseEnum>("axis");
    switch (axis)
    {
      case 0:
      case 1:
      case 2:
        _axis = axis;
        break;
      case 3:
      case 4:
      case 5:
        _axis = axis - 3;
        break;
    }
    _has_axis = true;
  }
}

Real
Piecewise::functionSize()
{
  return _linear_interp->getSampleSize();
}

Real
Piecewise::domain(int i)
{
  return _linear_interp->domain(i);
}

Real
Piecewise::range(int i)
{
  return _linear_interp->range(i);
}

std::pair<std::vector<Real>, std::vector<Real>>
Piecewise::buildFromFile()
{
  // Input parameters
  const FileName & data_file_name = getParam<FileName>("data_file");
  const MooseEnum & format = getParam<MooseEnum>("format");
  unsigned int x_index = getParam<unsigned int>("x_index_in_file");
  unsigned int y_index = getParam<unsigned int>("y_index_in_file");
  bool xy_only = getParam<bool>("xy_in_file_only");

  // Check that other forms of input are not set.
  if (isParamValid("x") || isParamValid("y") || isParamValid("xy_data"))
    mooseError("In Piecewise ",
               _name,
               ": Cannot specify 'data_file' and 'x', 'y', or 'xy_data' together.");

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
  return std::make_pair(reader.getData(x_index), reader.getData(y_index));
}

std::pair<std::vector<Real>, std::vector<Real>>
Piecewise::buildFromXandY()
{
  if (!isParamValid("x") || !isParamValid("y"))
    mooseError(
        "In Piecewise ", _name, ": Both 'x' and 'y' must be specified if either one is specified.");

  if (isParamValid("xy_data"))
    mooseError("In Piecewise ", _name, ": Cannot specify 'x', 'y', and 'xy_data' together.");

  return std::make_pair(getParam<std::vector<Real>>("x"), getParam<std::vector<Real>>("y"));
}

std::pair<std::vector<Real>, std::vector<Real>>
Piecewise::buildFromXY()
{
  std::vector<Real> xy = getParam<std::vector<Real>>("xy_data");
  unsigned int xy_size = xy.size();
  if (xy_size % 2 != 0)
    mooseError(
        "In Piecewise ", _name, ": Length of data provided in 'xy_data' must be a multiple of 2.");

  unsigned int data_size = xy_size / 2;
  std::vector<Real> x;
  std::vector<Real> y;
  x.reserve(data_size);
  y.reserve(data_size);
  for (unsigned int i = 0; i < xy_size; i += 2)
  {
    x.push_back(xy[i]);
    y.push_back(xy[i + 1]);
  }
  return std::make_pair(x, y);
}
