//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for detailss
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseMultiInterpolationFromReporter.h"
#include "GriddedData.h"

InputParameters
PiecewiseMultiInterpolationFromReporter::validParams()
{
  InputParameters params = Function::validParams();

  params.addClassDescription(
      "Apply a point load defined by Reporter. This is a copy from PiecewiseMultiInterpolation.");

  params.addRequiredParam<ReporterName>(
      "x_coord_name",
      "reporter x-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "y_coord_name",
      "reporter y-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "z_coord_name",
      "reporter z-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "time_name", "reporter time name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "value_name", "reporter value name.  This uses the reporter syntax <reporter>/<name>.");

  return params;
}

PiecewiseMultiInterpolationFromReporter::PiecewiseMultiInterpolationFromReporter(
    const InputParameters & parameters)
  : Function(parameters),
    ReporterInterface(this),
    _values(getReporterValue<std::vector<Real>>("value_name", REPORTER_MODE_REPLICATED)),
    _x_coord(getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)),
    _y_coord(getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)),
    _z_coord(getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED)),
    _time(getReporterValue<std::vector<Real>>("time_name", REPORTER_MODE_REPLICATED))
{
}

PiecewiseMultiInterpolationFromReporter::~PiecewiseMultiInterpolationFromReporter() {}

template <bool is_ad>
MooseADWrapper<PiecewiseMultiInterpolationFromReporter::GridPoint, is_ad>
PiecewiseMultiInterpolationFromReporter::pointInGrid(const MooseADWrapper<Real, is_ad> & t,
                                                     const MooseADWrapper<Point, is_ad> & p) const
{
  // convert the inputs to an input to the sample function using _axes
  MooseADWrapper<GridPoint, is_ad> point_in_grid(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
  {
    if (_axes[i] < 3)
      point_in_grid[i] = p(_axes[i]);
    else if (_axes[i] == 3) // the time direction
      point_in_grid[i] = t;
  }
  return point_in_grid;
}

template PiecewiseMultiInterpolationFromReporter::GridPoint
PiecewiseMultiInterpolationFromReporter::pointInGrid<false>(const Real &, const Point &) const;
template PiecewiseMultiInterpolationFromReporter::ADGridPoint
PiecewiseMultiInterpolationFromReporter::pointInGrid<true>(const ADReal &, const ADPoint &) const;

Real
PiecewiseMultiInterpolationFromReporter::value(Real t, const Point & p) const
{
  return sample(pointInGrid<false>(t, p));
}

ADReal
PiecewiseMultiInterpolationFromReporter::value(const ADReal & t, const ADPoint & p) const
{
  return sample(pointInGrid<true>(t, p));
}

ADReal
PiecewiseMultiInterpolationFromReporter::sample(const ADGridPoint &) const
{
  mooseError("The AD variant of 'sample' needs to be implemented");
}

void
PiecewiseMultiInterpolationFromReporter::getNeighborIndices(std::vector<Real> in_arr,
                                                            Real x,
                                                            unsigned int & lower_x,
                                                            unsigned int & upper_x) const
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

void
PiecewiseMultiInterpolationFromReporter::initialSetup()
{
  _gridded_data = (std::make_unique<GriddedData>(_x_coord, _y_coord, _z_coord, _time, _values));
  _dim = (_gridded_data->getDim());
  _gridded_data->getAxes(_axes);
  _gridded_data->getGrid(_grid);
  for (unsigned int i = 0; i < _dim; ++i)
    for (unsigned int j = 1; j < _grid[i].size(); ++j)
      if (_grid[i][j - 1] >= _grid[i][j])
        mooseError("PiecewiseMultiInterpolationFromReporter needs monotonically-increasing axis "
                   "data.  Axis ",
                   i,
                   " contains non-monotonicity at value ",
                   _grid[i][j]);
  std::set<int> s(_axes.begin(), _axes.end());
  if (s.size() != _dim)
    mooseError("PiecewiseMultiInterpolationFromReporter needs the AXES to be independent.  Check "
               "the AXIS lines in "
               "your data file.");
}
