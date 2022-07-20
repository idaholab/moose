//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterNearestPointAux.h"

registerMooseObject("isopodApp", ReporterNearestPointAux);

InputParameters
ReporterNearestPointAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Assigns variable based on the nearest point to coordinates and values defined by a vector "
      "reporter values, interpolates linearly in time with transient data.");
  params.addParam<ReporterName>(
      "coord_x",
      "Vector reporter value containing x-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>(
      "coord_y",
      "Vector reporter value containing y-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>(
      "coord_z",
      "Vector reporter value containing z-coordinate of points, default is assumed to be all 0s.");
  params.addParam<ReporterName>("time",
                                "Vector reporter value of time points if 'value' is a vector of "
                                "vectors with time-dependent data.");
  params.addRequiredParam<ReporterName>("value", "Reporter containing value data.");
  return params;
}

ReporterNearestPointAux::ReporterNearestPointAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    ReporterInterface(this),
    _coordx(isParamValid("coord_x") ? getReporterValue<std::vector<Real>>("coord_x") : _empty_vec),
    _coordy(isParamValid("coord_y") ? getReporterValue<std::vector<Real>>("coord_y") : _empty_vec),
    _coordz(isParamValid("coord_z") ? getReporterValue<std::vector<Real>>("coord_z") : _empty_vec),
    _coordt(isParamValid("time") ? getReporterValue<std::vector<Real>>("time") : _empty_vec),
    _values(getReporterValue<std::vector<Real>>("value"))
{
}

void
ReporterNearestPointAux::subdomainSetup()
{
  // Do some size checks
  const std::size_t nval = _values.size();
  if (nval == 0)
    paramError("value", "Number of values must be greater than 0.");
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

  // Find times for each unique coordinate
  std::map<Point, std::vector<std::pair<Real, Real>>> coord_mapping;
  for (const auto & i : make_range(nval))
  {
    Point pt;
    pt(0) = _coordx.empty() ? 0.0 : _coordx[i];
    pt(1) = _coordy.empty() ? 0.0 : _coordy[i];
    pt(2) = _coordz.empty() ? 0.0 : _coordz[i];
    const Real time = _coordt.empty() ? 0.0 : _coordt[i];

    std::vector<std::pair<Real, Real>> * vec = nullptr;
    for (auto & it : coord_mapping)
      if (pt.absolute_fuzzy_equals(it.first))
      {
        vec = &it.second;
        break;
      }
    if (!vec)
      vec = &coord_mapping[pt];

    vec->emplace_back(time, _values[i]);
    std::sort(vec->begin(),
              vec->end(),
              [](const std::pair<Real, Real> & a, const std::pair<Real, Real> & b)
              { return a.first < b.first; });
  }

  // Linearly interpolate in time
  _current_data.clear();
  for (const auto & it : coord_mapping)
  {
    auto & val = _current_data[it.first];
    const auto & tval = it.second;
    if (tval.size() == 1)
      val = tval[0].second;
    else if (MooseUtils::absoluteFuzzyLessEqual(_t, tval[0].first))
      val = tval[0].second;
    else if (MooseUtils::absoluteFuzzyGreaterEqual(_t, tval.back().first))
      val = tval.back().second;
    else
      for (std::size_t ti = 1; ti < tval.size(); ++ti)
      {
        if (MooseUtils::absoluteFuzzyEqual(_t, tval[ti].first))
        {
          val = tval[ti].second;
          break;
        }
        else if (tval[ti].first > _t)
        {
          const Real told = tval[ti - 1].first;
          const Real tnew = tval[ti].first;
          const Real vold = tval[ti - 1].second;
          const Real vnew = tval[ti].second;
          val = vold + (vnew - vold) * (_t - told) / (tnew - told);
          break;
        }
      }
  }
}

Real
ReporterNearestPointAux::computeValue()
{
  const Point & pt = _q_point[_qp];
  const auto it =
      std::min_element(_current_data.begin(),
                       _current_data.end(),
                       [&pt](const std::pair<Point, Real> & p1, const std::pair<Point, Real> & p2)
                       { return (pt - p1.first).norm_sq() < (pt - p2.first).norm_sq(); });
  return it->second;
}
