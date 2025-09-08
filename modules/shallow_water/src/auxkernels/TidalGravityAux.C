//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TidalGravityAux.h"
// Standard
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

registerMooseObject("ShallowWaterApp", TidalGravityAux);

InputParameters
TidalGravityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Scalar gravity field with simple Sun/Moon tidal correction projected onto local vertical.");

  params.addParam<Real>("g0", 9.81, "Base gravitational magnitude [m/s^2]");
  params.addParam<bool>("enable_tides", true, "Enable Sun/Moon tidal correction");
  params.addParam<Real>(
      "simulation_start_epoch",
      0.0,
      "Absolute UTC time (s since 1970-01-01) corresponding to simulation t=0 (overridden by start_datetime if provided)");
  params.addParam<std::string>(
      "start_datetime",
      "",
      "Start date/time in UTC for t=0 (e.g., '2025-01-01 00:00:00' or '2025-01-01T00:00:00Z'). Overrides simulation_start_epoch if set.");
  params.addParam<Real>("earth_radius", 6.371e6, "Earth mean radius [m]");
  params.addParam<Real>("mu_sun", 1.32712440018e20, "GM_sun [m^3/s^2]");
  params.addParam<Real>("sun_distance", 1.495978707e11, "Mean Sun-Earth distance [m]");
  params.addParam<Real>("sun_period", 365.25 * 86400.0, "Earth orbital period [s]");
  params.addParam<Real>("mu_moon", 4.9048695e12, "GM_moon [m^3/s^2]");
  params.addParam<Real>("moon_distance", 3.844e8, "Mean Moon-Earth distance [m]");
  params.addParam<Real>("moon_period", 27.321661 * 86400.0, "Moon sidereal orbital period [s]");
  return params;
}

TidalGravityAux::TidalGravityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _g0(getParam<Real>("g0")),
    _enable_tides(getParam<bool>("enable_tides")),
    _earth_radius(getParam<Real>("earth_radius")),
    _t0_epoch(getParam<Real>("simulation_start_epoch")),
    _start_datetime(getParam<std::string>("start_datetime")),
    _mu_sun(getParam<Real>("mu_sun")),
    _sun_distance(getParam<Real>("sun_distance")),
    _sun_period(getParam<Real>("sun_period")),
    _mu_moon(getParam<Real>("mu_moon")),
    _moon_distance(getParam<Real>("moon_distance")),
    _moon_period(getParam<Real>("moon_period")),
    _cached_t_step(std::numeric_limits<unsigned int>::max()),
    _a_tide(0.0, 0.0, 0.0)
{
  if (!_start_datetime.empty())
  {
    try
    {
      _t0_epoch = static_cast<Real>(parseStartDatetimeUTC(_start_datetime));
    }
    catch (const std::exception & e)
    {
      mooseWarning("TidalGravityAux failed to parse start_datetime='", _start_datetime,
                   "' — using simulation_start_epoch instead. Error: ", e.what());
    }
  }
}

std::time_t
TidalGravityAux::parseStartDatetimeUTC(const std::string & s) const
{
  // Accept formats: "YYYY-MM-DD HH:MM:SS" or "YYYY-MM-DDTHH:MM:SS" with optional trailing 'Z'
  std::string str = s;
  auto ltrim = [](std::string & x)
  {
    x.erase(x.begin(), std::find_if(x.begin(), x.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  };
  auto rtrim = [](std::string & x)
  {
    x.erase(std::find_if(x.rbegin(), x.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), x.end());
  };
  ltrim(str);
  rtrim(str);
  if (!str.empty() && (str.back() == 'Z' || str.back() == 'z'))
    str.pop_back();
  for (auto & ch : str)
    if (ch == 'T')
      ch = ' ';

  std::tm tm = {};
  std::istringstream iss(str);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (iss.fail())
    throw std::runtime_error("Invalid datetime format (expected YYYY-MM-DD HH:MM:SS or YYYY-MM-DDTHH:MM:SSZ)");

#if defined(_WIN32)
  std::time_t t = _mkgmtime(&tm);
#else
  std::time_t t = timegm(&tm);
#endif
  if (t == static_cast<std::time_t>(-1))
    throw std::runtime_error("Failed to convert datetime to epoch");
  return t;
}

void
TidalGravityAux::updateTidal()
{
  const Real t_abs = _t0_epoch + _t; // seconds UTC

  auto deg2rad = [](Real d) { return d * (libMesh::pi / 180.0); };
  auto rad2deg = [](Real r) { return r * (180.0 / libMesh::pi); };
  auto norm2pi = [&](Real ang) {
    while (ang < 0.0)
      ang += 2.0 * libMesh::pi;
    while (ang >= 2.0 * libMesh::pi)
      ang -= 2.0 * libMesh::pi;
    return ang;
  };

  // Epoch seconds -> Julian Day and centuries from J2000
  const Real JD = 2440587.5 + t_abs / 86400.0;
  const Real T = (JD - 2451545.0) / 36525.0;

  // Mean obliquity (deg)
  const Real eps_deg = 23.439291 - 0.0130042 * T;
  const Real eps = deg2rad(eps_deg);

  // --- Sun approximate ecliptic longitude and distance variation ---
  Real M_sun = deg2rad(357.52911 + 35999.05029 * T - 0.0001537 * T * T);
  Real L0 = deg2rad(280.46646 + 36000.76983 * T + 0.0003032 * T * T);
  Real C = deg2rad((1.914602 - 0.004817 * T - 0.000014 * T * T) * std::sin(M_sun) +
                   (0.019993 - 0.000101 * T) * std::sin(2 * M_sun) + 0.000289 * std::sin(3 * M_sun));
  Real lambda_sun = norm2pi(L0 + C);
  // Distance (AU) approximation
  Real R_AU = 1.00014 - 0.01671 * std::cos(M_sun) - 0.00014 * std::cos(2 * M_sun);

  // Sun unit vector in equatorial coordinates (ECI)
  const Real sx = std::cos(lambda_sun);
  const Real sy = std::cos(eps) * std::sin(lambda_sun);
  const Real sz = std::sin(eps) * std::sin(lambda_sun);
  RealVectorValue s_hat(sx, sy, sz);
  const Real s_norm = std::sqrt(sx * sx + sy * sy + sz * sz);
  if (s_norm > 0.0)
    s_hat /= s_norm;

  // --- Moon approximate ecliptic longitude/latitude and distance ---
  // Low-order series adapted from Meeus (coarse but sufficient for direction)
  Real Lp = deg2rad(218.3164477 + 481267.88123421 * T - 0.0015786 * T * T);
  Real D = deg2rad(297.8501921 + 445267.1114034 * T - 0.0018819 * T * T);
  Real M = M_sun;
  Real Mp = deg2rad(134.9633964 + 477198.8675055 * T + 0.0087414 * T * T);
  Real F = deg2rad(93.2720950 + 483202.0175233 * T - 0.0036539 * T * T);

  // Ecliptic longitude and latitude corrections (degrees -> radians on the sum)
  Real lon_corr_deg = 6.289 * std::sin(Mp) + 1.274 * std::sin(2 * D - Mp) +
                      0.658 * std::sin(2 * D) + 0.213 * std::sin(2 * Mp) - 0.186 * std::sin(M) -
                      0.059 * std::sin(2 * D - 2 * Mp) - 0.057 * std::sin(2 * D - M - Mp) +
                      0.053 * std::sin(2 * D + Mp) + 0.046 * std::sin(2 * D - M) +
                      0.041 * std::sin(M - Mp);
  Real lat_corr_deg = 5.128 * std::sin(F) + 0.280 * std::sin(Mp + F) + 0.277 * std::sin(Mp - F) +
                      0.173 * std::sin(2 * D - F) + 0.055 * std::sin(2 * D + F) +
                      0.046 * std::sin(2 * D - Mp + F) + 0.033 * std::sin(2 * D - Mp - F) +
                      0.017 * std::sin(2 * Mp + F);

  Real lambda_moon = norm2pi(Lp + deg2rad(lon_corr_deg));
  Real beta_moon = deg2rad(lat_corr_deg);

  // Approximate Moon distance (meters) with mean distance (slight variation ignored)
  Real r_moon = _moon_distance; // could enhance with periodic terms

  // Convert Moon ecliptic spherical to equatorial vector
  Real mx = std::cos(beta_moon) * std::cos(lambda_moon);
  Real my = std::cos(eps) * std::cos(beta_moon) * std::sin(lambda_moon) -
            std::sin(eps) * std::sin(beta_moon);
  Real mz = std::sin(eps) * std::cos(beta_moon) * std::sin(lambda_moon) +
            std::cos(eps) * std::sin(beta_moon);
  RealVectorValue m_hat(mx, my, mz);
  const Real m_norm = std::sqrt(mx * mx + my * my + mz * mz);
  if (m_norm > 0.0)
    m_hat /= m_norm;

  // Amplitudes (use distances for scaling; Sun scaled by AU to meters)
  const Real AU = 1.495978707e11;
  const Real r_sun = R_AU * AU;
  const Real a_sun = 2.0 * _mu_sun * _earth_radius / (r_sun * r_sun * r_sun);
  const Real a_moon = 2.0 * _mu_moon * _earth_radius / (r_moon * r_moon * r_moon);

  _a_tide = (_enable_tides ? (a_sun * s_hat + a_moon * m_hat) : RealVectorValue(0, 0, 0));
}

Real
TidalGravityAux::computeValue()
{
  if (_cached_t_step != _t_step)
  {
    updateTidal();
    _cached_t_step = _t_step;
  }
  const Real rmag = _q_point[_qp].norm();
  RealVectorValue n_r(0, 0, 1);
  if (rmag > 0.0)
    n_r = _q_point[_qp] / rmag;
  const RealVectorValue gvec = (-_g0) * n_r + _a_tide;
  return -gvec * n_r;
}
