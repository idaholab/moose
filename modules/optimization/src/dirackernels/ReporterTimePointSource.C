//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterTimePointSource.h"
#include "MooseUtils.h"

registerMooseObject("OptimizationApp", ReporterTimePointSource);

InputParameters
ReporterTimePointSource::validParams()
{
  InputParameters params = ReporterPointSource::validParams();
  params.addClassDescription("Apply a time dependent point load defined by Reporters.");
  params.addParam<ReporterName>("time_name",
                                "Name of vector-postprocessor or reporter vector containing time, "
                                "default is assumed to be all 0s.");
  params.addParam<Real>("reverse_time_end", 0.0, "End time used for reversing the time values.");
  return params;
}

ReporterTimePointSource::ReporterTimePointSource(const InputParameters & parameters)
  : ReporterPointSource(parameters),
    _coordt(isParamValid("time_name") ? getReporterValue<std::vector<Real>>("time_name")
                                      : _zeros_vec),
    _reverse_time_end(getParam<Real>("reverse_time_end"))
{
}

void
ReporterTimePointSource::addPoints()
{
  // Do some size checks
  const auto nval = _values.size();
  if (nval == 0)
    paramError("value_name", "Value vector must not be empty.");

  // resize these incase the values reporters changed size
  // this will only change data constructed to reference these
  _ones_vec.resize(nval, 1.0);
  _zeros_vec.resize(nval, 0.0);
  _zeros_pts.resize(nval, Point());

  errorCheck("x_coord_name", _coordx.size());
  errorCheck("y_coord_name", _coordy.size());
  errorCheck("z_coord_name", _coordz.size());
  errorCheck("weight_name", _weight.size());
  errorCheck("point_name", _point.size());
  errorCheck("time_name", _coordt.size());

  _point_to_weightedValue.clear();
  const Real at =
      MooseUtils::absoluteFuzzyEqual(_reverse_time_end, 0.0) ? _t : _reverse_time_end - _t + _dt;
  for (const auto & i : make_range(nval))
  {
    if (_coordt.empty() || MooseUtils::absoluteFuzzyEqual(at, _coordt[i]))
    {
      if (_read_in_points)
      {
        fillPoint(_point[i], i);
      }
      else
      {
        const Point point = Point(_coordx[i], _coordy[i], _coordz[i]);
        fillPoint(point, i);
      }
    }
  }
}
