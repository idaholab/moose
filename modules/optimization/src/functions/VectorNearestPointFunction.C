//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorNearestPointFunction.h"

registerMooseObject("isopodApp", VectorNearestPointFunction);

InputParameters
VectorNearestPointFunction::validParams()
{
  InputParameters params = OptimizationFunction::validParams();
  params.addClassDescription(
      "Function based on the nearest point to coordinates and values defined by a vector "
      "values, interpolates linearly in time with transient data.");
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
  return params;
}

VectorNearestPointFunction::VectorNearestPointFunction(const InputParameters & parameters)
  : OptimizationFunction(parameters),
    ReporterInterface(this),
    _coordx(isParamValid("coord_x") ? getReporterValue<std::vector<Real>>("coord_x") : _empty_vec),
    _coordy(isParamValid("coord_y") ? getReporterValue<std::vector<Real>>("coord_y") : _empty_vec),
    _coordz(isParamValid("coord_z") ? getReporterValue<std::vector<Real>>("coord_z") : _empty_vec),
    _coordt(isParamValid("time") ? getReporterValue<std::vector<Real>>("time") : _empty_vec),
    _values(getReporterValue<std::vector<Real>>("value"))
{
}

Real
VectorNearestPointFunction::value(Real t, const Point & p) const
{
  const std::array<std::pair<Real, std::size_t>, 2> tv = findNearestPoint(t, p);

  if (tv[0].second == tv[1].second)
    return _values[tv[0].second];

  const Real told = tv[0].first;
  const Real tnew = tv[1].first;
  const Real vold = _values[tv[0].second];
  const Real vnew = _values[tv[1].second];
  return vold + (vnew - vold) * (t - told) / (tnew - told);
}

RealGradient
VectorNearestPointFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  return RealGradient(0, 0, 0);
}

Real
VectorNearestPointFunction::timeDerivative(Real t, const Point & p) const
{
  const std::array<std::pair<Real, std::size_t>, 2> tv = findNearestPoint(t, p);

  if (tv[0].second == tv[1].second)
    return 0.0;

  const Real told = tv[0].first;
  const Real tnew = tv[1].first;
  const Real vold = _values[tv[0].second];
  const Real vnew = _values[tv[0].second];
  return (vnew - vold) / (tnew - told);
}

std::vector<Real>
VectorNearestPointFunction::parameterGradient(Real t, const Point & p) const
{
  const std::array<std::pair<Real, std::size_t>, 2> tv = findNearestPoint(t, p);
  std::vector<Real> pd(_nval, 0.0);

  if (tv[0].second == tv[1].second)
    pd[tv[0].second] = 1;
  else
  {
    const Real told = tv[0].first;
    const Real tnew = tv[1].first;
    pd[tv[0].second] = (tnew - t) / (tnew - told);
    pd[tv[1].second] = (t - told) / (tnew - told);
  }
  return pd;
}

void
VectorNearestPointFunction::buildCoordinateMapping() const
{
  // Do some size checks
  _nval = std::max({_coordx.size(), _coordy.size(), _coordz.size(), _coordt.size()});
  if (_nval == 0)
    paramError("value", "At least one coordinate vector must not be empty.");
  else if (!_coordx.empty() && _coordx.size() != _nval)
    paramError("coord_x",
               "Number of x coordinates (",
               _coordx.size(),
               ") does not match number of values (",
               _nval,
               ").");
  else if (!_coordy.empty() && _coordy.size() != _nval)
    paramError("coord_y",
               "Number of y coordinates (",
               _coordy.size(),
               ") does not match number of values (",
               _nval,
               ").");
  else if (!_coordz.empty() && _coordz.size() != _nval)
    paramError("coord_z",
               "Number of z coordinates (",
               _coordz.size(),
               ") does not match number of values (",
               _nval,
               ").");
  else if (!_coordt.empty() && _coordt.size() != _nval)
    paramError("time",
               "Number of times (",
               _coordt.size(),
               ") does not match number of values (",
               _nval,
               ").");

  // Find times for each unique coordinate
  _coord_mapping.clear();
  for (const auto & i : make_range(_nval))
  {
    Point pt;
    pt(0) = _coordx.empty() ? 0.0 : _coordx[i];
    pt(1) = _coordy.empty() ? 0.0 : _coordy[i];
    pt(2) = _coordz.empty() ? 0.0 : _coordz[i];
    const Real time = _coordt.empty() ? 0.0 : _coordt[i];

    std::vector<std::pair<Real, std::size_t>> * vec = nullptr;
    for (auto & it : _coord_mapping)
      if (pt.absolute_fuzzy_equals(it.first))
      {
        vec = &it.second;
        break;
      }
    if (!vec)
      vec = &_coord_mapping[pt];

    vec->emplace_back(time, i);
    std::sort(vec->begin(),
              vec->end(),
              [](const std::pair<Real, Real> & a, const std::pair<Real, Real> & b)
              { return a.first < b.first; });
  }
}

std::array<std::pair<Real, std::size_t>, 2>
VectorNearestPointFunction::findNearestPoint(Real t, const Point & p) const
{
  if (_coord_mapping.empty())
    buildCoordinateMapping();

  // Make sure values is correct size
  if (_values.size() != _nval)
    paramError("value",
               "Size of value vector (",
               _values.size(),
               ") does not match number of coordinates specified (",
               _nval,
               ").");

  const auto & tval =
      std::min_element(_coord_mapping.begin(),
                       _coord_mapping.end(),
                       [&p](const std::pair<Point, std::vector<std::pair<Real, std::size_t>>> & p1,
                            const std::pair<Point, std::vector<std::pair<Real, std::size_t>>> & p2)
                       { return (p - p1.first).norm_sq() < (p - p2.first).norm_sq(); })
          ->second;

  if (tval.size() == 1 || MooseUtils::absoluteFuzzyLessEqual(t, tval[0].first))
    return {tval[0], tval[0]};
  else if (MooseUtils::absoluteFuzzyGreaterEqual(t, tval.back().first))
    return {tval.back(), tval.back()};
  else
    for (std::size_t ti = 1; ti < tval.size(); ++ti)
      if (MooseUtils::absoluteFuzzyGreaterEqual(tval[ti].first, t))
        return {tval[ti - 1], tval[ti]};
  return std::array<std::pair<Real, std::size_t>, 2>();
}
