//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseMultiInterpolation.h"
#include "GriddedData.h"

template <>
InputParameters
validParams<PiecewiseMultiInterpolation>()
{
  InputParameters params = validParams<Function>();
  params.addParam<FileName>(
      "data_file",
      "File holding data for use with PiecewiseMultiInterpolation.  Format: any empty line and any "
      "line "
      "beginning with # are ignored, all other lines are assumed to contain relevant information.  "
      "The file must begin with specification of the grid.  This is done through lines containing "
      "the keywords: AXIS X; AXIS Y; AXIS Z; or AXIS T.  Immediately following the keyword line "
      "must be a space-separated line of real numbers which define the grid along the specified "
      "axis.  These data must be monotonically increasing.  After all the axes and their grids "
      "have been specified, there must be a line that is DATA.  Following that line, function "
      "values are given in the correct order (they may be on individual lines, or be "
      "space-separated on a number of lines).  When the function is evaluated, f[i,j,k,l] "
      "corresponds to the i + j*Ni + k*Ni*Nj + l*Ni*Nj*Nk data value.  Here i>=0 corresponding to "
      "the index along the first AXIS, j>=0 corresponding to the index along the second AXIS, etc, "
      "and Ni = number of grid points along the first AXIS, etc.");
  return params;
}

PiecewiseMultiInterpolation::PiecewiseMultiInterpolation(const InputParameters & parameters)
  : Function(parameters),
    _gridded_data(libmesh_make_unique<GriddedData>(getParam<FileName>("data_file"))),
    _dim(_gridded_data->getDim())
{
  _gridded_data->getAxes(_axes);
  _gridded_data->getGrid(_grid);

  // GriddedData does not require monotonicity of axes, but we do
  for (unsigned int i = 0; i < _dim; ++i)
    for (unsigned int j = 1; j < _grid[i].size(); ++j)
      if (_grid[i][j - 1] >= _grid[i][j])
        mooseError("PiecewiseMultiInterpolation needs monotonically-increasing axis data.  Axis ",
                   i,
                   " contains non-monotonicity at value ",
                   _grid[i][j]);

  // GriddedData does not demand that each axis is independent, but we do
  std::set<int> s(_axes.begin(), _axes.end());
  if (s.size() != _dim)
    mooseError(
        "PiecewiseMultiInterpolation needs the AXES to be independent.  Check the AXIS lines in "
        "your data file.");
}

PiecewiseMultiInterpolation::~PiecewiseMultiInterpolation() {}

Real
PiecewiseMultiInterpolation::value(Real t, const Point & p)
{
  // convert the inputs to an input to the sample function using _axes
  std::vector<Real> pt_in_grid(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
  {
    if (_axes[i] < 3)
      pt_in_grid[i] = p(_axes[i]);
    else if (_axes[i] == 3) // the time direction
      pt_in_grid[i] = t;
  }
  return sample(pt_in_grid);
}

void
PiecewiseMultiInterpolation::getNeighborIndices(std::vector<Real> in_arr,
                                                Real x,
                                                unsigned int & lower_x,
                                                unsigned int & upper_x)
{
  int N = in_arr.size();
  if (x <= in_arr[0])
  {
    lower_x = 0;
    upper_x = 0;
  }
  else if (x >= in_arr[N - 1])
  {
    lower_x = N - 1;
    upper_x = N - 1;
  }
  else
  {
    // returns up which points at the first element in inArr that is not less than x
    std::vector<double>::iterator up = std::lower_bound(in_arr.begin(), in_arr.end(), x);

    // std::distance returns std::difference_type, which can be negative in theory, but
    // in this context will always be >=0.  Therefore the explicit cast is just to shut
    // the compiler up.
    upper_x = static_cast<unsigned int>(std::distance(in_arr.begin(), up));
    if (in_arr[upper_x] == x)
      lower_x = upper_x;
    else
      lower_x = upper_x - 1;
  }
}
