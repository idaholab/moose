//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseBilinear.h"
#include "ColumnMajorMatrix.h"
#include "BilinearInterpolation.h"
#include "MooseUtils.h"

#include <fstream>

registerMooseObject("MooseApp", PiecewiseBilinear);

InputParameters
PiecewiseBilinear::validParams()
{
  InputParameters params = Function::validParams();
  params.addParam<FileName>(
      "data_file", "", "File holding csv data for use with PiecewiseBilinear");
  params.addParam<std::vector<Real>>("x", "The x abscissa values");
  params.addParam<std::vector<Real>>("y", "The y abscissa values");
  params.addParam<std::vector<Real>>("z", "The ordinate values");
  params.addParam<int>("axis", -1, "The axis used (0, 1, or 2 for x, y, or z).");
  params.addParam<int>(
      "xaxis", -1, "The coordinate used for x-axis data (0, 1, or 2 for x, y, or z).");
  params.addParam<int>(
      "yaxis", -1, "The coordinate used for y-axis data (0, 1, or 2 for x, y, or z).");
  params.addParam<Real>(
      "scale_factor", 1.0, "Scale factor to be applied to the axis, yaxis, or xaxis values");
  params.addParam<bool>("radial",
                        false,
                        "Set to true if you want to interpolate along a radius "
                        "rather that along a specific axis, and note that you "
                        "have to define xaxis and yaxis in the input file");
  params.addClassDescription("Interpolates values from a csv file");
  return params;
}

PiecewiseBilinear::PiecewiseBilinear(const InputParameters & parameters)
  : Function(parameters),
    _data_file_name(getParam<FileName>("data_file")),
    _axis(getParam<int>("axis")),
    _yaxis(getParam<int>("yaxis")),
    _xaxis(getParam<int>("xaxis")),
    _axisValid(_axis > -1 && _axis < 3),
    _yaxisValid(_yaxis > -1 && _yaxis < 3),
    _xaxisValid(_xaxis > -1 && _xaxis < 3),
    _scale_factor(getParam<Real>("scale_factor")),
    _radial(getParam<bool>("radial"))
{

  if (!_axisValid && !_yaxisValid && !_xaxisValid)
    mooseError("In PiecewiseBilinear ",
               _name,
               ": None of axis, yaxis, or xaxis properly defined.  Allowable range is 0-2");

  if (_axisValid && (_yaxisValid || _xaxisValid))
    mooseError("In PiecewiseBilinear ", _name, ": Cannot define axis with either yaxis or xaxis");

  if (_radial && (!_yaxisValid || !_xaxisValid))
    mooseError(
        "In PiecewiseBilinear ", _name, ": yaxis and xaxis must be defined when radial = true");

  std::vector<Real> x;
  std::vector<Real> y;
  ColumnMajorMatrix z;
  std::vector<Real> z_vec;

  if (!_data_file_name.empty())
  {
    if (parameters.isParamValid("x") || parameters.isParamValid("y") ||
        parameters.isParamValid("z"))
      mooseError("In PiecewiseBilinear: Cannot specify 'data_file' and 'x', 'y', or 'z' together.");
    else
      parse(_data_file_name, x, y, z, name());
  }

  else if (!(parameters.isParamValid("x") && parameters.isParamValid("y") &&
             parameters.isParamValid("z")))
    mooseError("In PiecewiseBilinear: 'x' and 'y' and 'z' must be specified if any one is "
               "specified or no 'data_file' is specified.");

  else
  {
    x = getParam<std::vector<Real>>("x");
    y = getParam<std::vector<Real>>("y");
    z_vec = getParam<std::vector<Real>>("z");

    // check that size of z = (size of x)*(size of y)
    if (z_vec.size() != x.size() * y.size())
      mooseError("In PiecewiseBilinear: Size of z should be the size of x times the size of y.");

    // reshape and populate z matrix
    z.reshape(y.size(), x.size());
    int idx = 0;
    for (unsigned int i = 0; i < y.size(); i++)
      for (unsigned int j = 0; j < x.size(); j++)
      {
        z(i, j) = z_vec[idx];
        idx += 1;
      }
  }

  _bilinear_interp = std::make_unique<BilinearInterpolation>(x, y, z);
}

PiecewiseBilinear::~PiecewiseBilinear() {}

Real
PiecewiseBilinear::value(Real t, const Point & p) const
{
  return valueInternal(t, p);
}

ADReal
PiecewiseBilinear::value(const ADReal & t, const ADPoint & p) const
{
  return valueInternal(t, p);
}

template <typename T, typename P>
T
PiecewiseBilinear::valueInternal(T t, const P & p) const
{
  T retVal = 0.0;
  if (_yaxisValid && _xaxisValid && _radial)
  {
    const auto rx = p(_xaxis) * p(_xaxis);
    const auto ry = p(_yaxis) * p(_yaxis);
    const auto r = std::sqrt(rx + ry);
    retVal = _bilinear_interp->sample(r, t);
  }
  else if (_axisValid)
    retVal = _bilinear_interp->sample(p(_axis), t);
  else if (_yaxisValid && !_radial)
  {
    if (_xaxisValid)
      retVal = _bilinear_interp->sample(p(_xaxis), p(_yaxis));
    else
      retVal = _bilinear_interp->sample(t, p(_yaxis));
  }
  else
    retVal = _bilinear_interp->sample(p(_xaxis), t);

  return retVal * _scale_factor;
}

void
PiecewiseBilinear::parse(const std::string & data_file_name,
                         std::vector<Real> & x,
                         std::vector<Real> & y,
                         ColumnMajorMatrix & z,
                         const std::string & object_name)
{
  std::ifstream file(data_file_name.c_str());
  if (!file.good())
    ::mooseError(object_name, " : Error opening file '", data_file_name, "'.");

  std::size_t num_lines = 0;
  std::size_t num_cols = libMesh::invalid_uint;
  std::vector<Real> data;

  std::string line;
  std::vector<Real> line_data;
  while (std::getline(file, line))
  {
    num_lines++;
    if (!MooseUtils::tokenizeAndConvert<double>(line, line_data, ", "))
      ::mooseError(object_name, " : Error parsing file '", data_file_name, "' on line ", num_lines);

    data.insert(data.end(), line_data.begin(), line_data.end());

    if (num_cols == libMesh::invalid_uint)
      num_cols = line_data.size();
    else if (line_data.size() != num_cols + 1)
      ::mooseError(object_name,
                   " : Read ",
                   line_data.size(),
                   " columns of data but expected ",
                   num_cols + 1,
                   " columns in line ",
                   num_lines);
  }

  x.resize(num_cols);
  y.resize(num_lines - 1);
  z.reshape(num_lines - 1, num_cols);
  std::size_t offset = 0;

  // Extract the first line's data (the x axis data)
  for (unsigned int j = 0; j < num_cols; ++j)
    x[j] = data[offset++];

  for (unsigned int i = 0; i < num_lines - 1; ++i)
  {
    // Extract the y axis entry for this line
    y[i] = data[offset++];

    // Extract the function values for this row in the matrix
    for (unsigned int j = 0; j < num_cols; ++j)
      z(i, j) = data[offset++];
  }

  if (data.size() != offset)
    ::mooseError(object_name, " : Inconsistency in data read from '", data_file_name, "'.");
}
