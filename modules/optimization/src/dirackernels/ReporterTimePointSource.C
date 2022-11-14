//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  InputParameters params = DiracKernel::validParams();
  params.addClassDescription("Apply a point load defined by vectors.");
  params.addParam<ReporterName>("x_coord_name",
                                "Name of vector-postprocessor or reporter vector containing "
                                "x-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>("y_coord_name",
                                "Name of vector-postprocessor or reporter vector containing "
                                "y-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>("z_coord_name",
                                "Name of vector-postprocessor or reporter vector containing "
                                "z-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>("time_name",
                                "Name of vector-postprocessor or reporter vector containing time, "
                                "default is assumed to be all 0s.");
  params.addRequiredParam<ReporterName>(
      "value_name", "Name of vector-postprocessor or reporter vector containing value data.");
  params.addParam<Real>("reverse_time_end", 0.0, "End time used for reversing the time values.");
  return params;
}

ReporterTimePointSource::ReporterTimePointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _coordx(isParamValid("x_coord_name") ? getReporterValue<std::vector<Real>>("x_coord_name")
                                         : _empty_vec),
    _coordy(isParamValid("y_coord_name") ? getReporterValue<std::vector<Real>>("y_coord_name")
                                         : _empty_vec),
    _coordz(isParamValid("z_coord_name") ? getReporterValue<std::vector<Real>>("z_coord_name")
                                         : _empty_vec),
    _coordt(isParamValid("time_name") ? getReporterValue<std::vector<Real>>("time_name")
                                      : _empty_vec),
    _values(getReporterValue<std::vector<Real>>("value_name")),
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
  else if (!_coordx.empty() && _coordx.size() != nval)
    paramError("x_coord_name",
               "Number of x coordinates (",
               _coordx.size(),
               ") does not match number of values (",
               nval,
               ").");
  else if (!_coordy.empty() && _coordy.size() != nval)
    paramError("y_coord_name",
               "Number of y coordinates (",
               _coordy.size(),
               ") does not match number of values (",
               nval,
               ").");
  else if (!_coordz.empty() && _coordz.size() != nval)
    paramError("z_coord_name",
               "Number of z coordinates (",
               _coordz.size(),
               ") does not match number of values (",
               nval,
               ").");
  else if (!_coordt.empty() && _coordt.size() != nval)
    paramError("time_name",
               "Number of times (",
               _coordt.size(),
               ") does not match number of values (",
               nval,
               ").");

  _point_to_index.clear();
  const Real at =
      MooseUtils::absoluteFuzzyEqual(_reverse_time_end, 0.0) ? _t : _reverse_time_end - _t + _dt;
  unsigned int id = 0;
  for (const auto & i : make_range(nval))
    if (_coordt.empty() || MooseUtils::absoluteFuzzyEqual(at, _coordt[i]))
    {
      Point pt;
      pt(0) = _coordx.empty() ? 0.0 : _coordx[i];
      pt(1) = _coordy.empty() ? 0.0 : _coordy[i];
      pt(2) = _coordz.empty() ? 0.0 : _coordz[i];
      _point_to_index[pt] = i;
      addPoint(pt, id++);
    }
}

Real
ReporterTimePointSource::computeQpResidual()
{
  return -_test[_i][_qp] * _values[_point_to_index[_current_point]];
}
