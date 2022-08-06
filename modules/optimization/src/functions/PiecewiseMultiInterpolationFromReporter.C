//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for detailss
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseMultiInterpolationFromReporter.h"

InputParameters
PiecewiseMultiInterpolationFromReporter::validParams()
{
  InputParameters params = Function::validParams();

  params.addClassDescription(
      "This is very similar to PiecewiseMultiInterpolation.  However, it uses a "
      "GriddedDataReporter to get its grid and axes data.  The values from at each grid point can "
      "be obtained from the GriddedDataReporter or a seperate reporter.");

  params.addRequiredParam<ReporterName>("grid_name",
                                        "reporter from GriddedDataReporter containing grid.  This "
                                        "uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>("axes_name",
                                        "reporter from GriddedDataReporter containing axes.  This "
                                        "uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "step_name",
      "reporter from GriddedDataReporter containing step vector for striding the grid.  This "
      "uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "dim_name",
      "reporter from GriddedDataReporter containing grid dimension.  This "
      "uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>("values_name",
                                        "reporter containing value for each point in the grid.  "
                                        "This uses the reporter syntax <reporter>/<name>.");

  return params;
}

PiecewiseMultiInterpolationFromReporter::PiecewiseMultiInterpolationFromReporter(
    const InputParameters & parameters)
  : Function(parameters),
    ReporterInterface(this),
    _grid(getReporterValue<std::vector<std::vector<Real>>>("grid_name", REPORTER_MODE_REPLICATED)),
    _axes(getReporterValue<std::vector<int>>("axes_name", REPORTER_MODE_REPLICATED)),
    _step(getReporterValue<std::vector<unsigned int>>("step_name", REPORTER_MODE_REPLICATED)),
    _dim(getReporterValue<unsigned int>("dim_name", REPORTER_MODE_REPLICATED)),
    _values(getReporterValue<std::vector<Real>>("values_name", REPORTER_MODE_REPLICATED))
{
}

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
               "the axes_name reporter in "
               "your GriddedDataReporter file.");
}

Real
PiecewiseMultiInterpolationFromReporter::evaluateFcn(const GridIndex & ijk) const
{
  unsigned int gridEntries = 1;
  for (unsigned int i = 0; i < _dim; ++i)
    gridEntries *= _grid[i].size();

  if (gridEntries != _values.size())
    mooseError("Gridded data evaluateFcn grid contains ",
               gridEntries,
               " grid entries, but the values_name reporter contains ",
               _values.size(),
               " entries.  These must be the same.");

  if (ijk.size() != _dim)
    mooseError(
        "Gridded data evaluateFcn called with ", ijk.size(), " arguments, but expected ", _dim);
  unsigned int index = ijk[0];
  for (unsigned int i = 1; i < _dim; ++i)
    index += ijk[i] * _step[i];
  if (index >= _values.size())
    mooseError("Gridded data evaluateFcn attempted to access index ",
               index,
               " of function, but it contains only ",
               _values.size(),
               " entries");
  return _values[index];
}
