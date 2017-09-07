/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

  MooseEnum format("rows columns", "rows");
  params.addParam<MooseEnum>(
      "format", format, "Format of csv data file that is in either in columns or rows");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");

  MooseEnum axis("0 1 2");
  params.addParam<MooseEnum>(
      "axis",
      axis,
      "The axis used (0, 1, or 2 for x, y, or z) if this is to be a function of position");
  return params;
}

Piecewise::Piecewise(const InputParameters & parameters)
  : Function(parameters), _scale_factor(getParam<Real>("scale_factor")), _has_axis(false)
{
  std::vector<Real> x;
  std::vector<Real> y;

  if (isParamValid("data_file"))
    buildFromFile(x, y);

  else if (isParamValid("x") || isParamValid("y"))
    buildFromXandY(x, y);

  else if (isParamValid("xy_data"))
    buildFromXY(x, y);

  else
    mooseError("In Piecewise ",
               _name,
               ": Either 'data_file', 'x' and 'y', or 'xy_data' must be specified.");

  setData(x, y);
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
    _axis = getParam<MooseEnum>("axis");
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

void
Piecewise::buildFromFile(std::vector<Real> & x, std::vector<Real> & y)
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
  reader.setFormat(format);
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
  x = reader.getData(x_index);
  y = reader.getData(y_index);
}

void
Piecewise::buildFromXandY(std::vector<Real> & x, std::vector<Real> & y)
{
  if (!isParamValid("x") || !isParamValid("y"))
    mooseError(
        "In Piecewise ", _name, ": Both 'x' and 'y' must be specified if either one is specified.");

  if (isParamValid("xy_data"))
    mooseError("In Piecewise ", _name, ": Cannot specify 'x', 'y', and 'xy_data' together.");

  x = getParam<std::vector<Real>>("x");
  y = getParam<std::vector<Real>>("y");
}

void
Piecewise::buildFromXY(std::vector<Real> & x, std::vector<Real> & y)
{
  std::vector<Real> xy = getParam<std::vector<Real>>("xy_data");
  unsigned int xy_size = xy.size();
  if (xy_size % 2 != 0)
    mooseError(
        "In Piecewise ", _name, ": Length of data provided in 'xy_data' must be a multiple of 2.");

  unsigned int x_size = xy_size / 2;
  x.reserve(x_size);
  y.reserve(x_size);
  for (unsigned int i = 0; i < xy_size / 2; ++i)
  {
    x.push_back(xy[i * 2]);
    y.push_back(xy[i * 2 + 1]);
  }
}
