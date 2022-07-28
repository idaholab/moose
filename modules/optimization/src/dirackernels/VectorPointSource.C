//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPointSource.h"
#include "MooseUtils.h"

registerMooseObject("isopodApp", VectorPointSource);

InputParameters
VectorPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addClassDescription("Apply a point load defined by vectors.");
  params.addParam<ReporterName>(
      "coord_x",
      "Vector value containing x-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>(
      "coord_y",
      "Vector value containing y-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>(
      "coord_z",
      "Vector value containing z-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>("time",
                                "Vector value containing time, default is assumed to be all 0s.");
  params.addRequiredParam<ReporterName>("value", "Reporter containing value data.");
  params.addParam<Real>("reverse_time_end", 0.0, "End time used for reversing the time values.");
  return params;
}

VectorPointSource::VectorPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _coordx(isParamValid("coord_x") ? getReporterValue<std::vector<Real>>("coord_x") : _empty_vec),
    _coordy(isParamValid("coord_y") ? getReporterValue<std::vector<Real>>("coord_y") : _empty_vec),
    _coordz(isParamValid("coord_z") ? getReporterValue<std::vector<Real>>("coord_z") : _empty_vec),
    _coordt(isParamValid("time") ? getReporterValue<std::vector<Real>>("time") : _empty_vec),
    _values(getReporterValue<std::vector<Real>>("value")),
    _reverse_time_end(getParam<Real>("reverse_time_end"))
{
}

void
VectorPointSource::addPoints()
{
  // Do some size checks
  const auto nval = _values.size();
  if (nval == 0)
    paramError("value", "Value vector must not be empty.");
  else if (!_coordx.empty() && _coordx.size() != nval)
    paramError("coord_x",
               "Number of x coordinates (",
               _coordx.size(),
               ") does not match number of values (",
               nval,
               ").");
  else if (!_coordy.empty() && _coordy.size() != nval)
    paramError("coord_y",
               "Number of y coordinates (",
               _coordy.size(),
               ") does not match number of values (",
               nval,
               ").");
  else if (!_coordz.empty() && _coordz.size() != nval)
    paramError("coord_z",
               "Number of z coordinates (",
               _coordz.size(),
               ") does not match number of values (",
               nval,
               ").");
  else if (!_coordt.empty() && _coordt.size() != nval)
    paramError("time",
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
VectorPointSource::computeQpResidual()
{
  return -_test[_i][_qp] * _values[_point_to_index[_current_point]];
}
