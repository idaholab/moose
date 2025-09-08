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
#include <chrono>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>

// Ephemeris helpers (C++17, self-contained)
namespace Moose
{
namespace astro
{

using clock_t = std::chrono::system_clock;
using nanos = std::chrono::nanoseconds;
using SysTimeNs = std::chrono::time_point<clock_t, nanos>;

const Real degrees = libMesh::pi / 180.0;

inline Real
wrap2pi(Real a)
{
  a = std::fmod(a, 2.0 * libMesh::pi);
  return a < 0 ? a + 2.0 * libMesh::pi : a;
}

inline Real
wrap360(Real d)
{
  d = std::fmod(d, 360.0);
  return d < 0 ? d + 360.0 : d;
}

inline Real
clamp(Real x, Real lo, Real hi)
{
  return x < lo ? lo : (x > hi ? hi : x);
}

// Howard Hinnant-style civil -> days since Unix epoch (1970-01-01)
// Works for proleptic Gregorian dates, valid for all Y/M/D.
static inline long long
daysFromCivil(int y, unsigned m, unsigned d)
{
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);           // [0, 399]
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;          // [0, 146096]
  return static_cast<long long>(era) * 146097 + static_cast<long long>(doe) - 719468;
}

// Parse with std::get_time("%Y-%m-%d %H:%M:%S"), apply offset_hours (local = UTC + offset),
// return Unix epoch seconds (UTC).
inline std::int64_t
toUnixEpoch(const std::string & datetime, double offset_hours)
{
  std::tm tm{};
  std::istringstream iss(datetime);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (iss.fail())
    throw std::runtime_error("Invalid datetime: " + datetime + " (expected %Y-%m-%d %H:%M:%S)");

  // Convert calendar to seconds since Unix epoch (treating parsed time as "local")
  const long long days = daysFromCivil(
      tm.tm_year + 1900, static_cast<unsigned>(tm.tm_mon + 1), static_cast<unsigned>(tm.tm_mday));
  long long seconds = days * 86400LL + tm.tm_hour * 3600LL + tm.tm_min * 60LL + tm.tm_sec;

  // UTC = local - offset  =>  epoch_utc = epoch_local - offset_seconds
  const long long offset_sec = static_cast<long long>(std::llround(offset_hours * 3600.0));
  return seconds - offset_sec;
}

inline Real
julianDateFromSystemTime(const SysTimeNs & tp_utc)
{
  using days_d = std::chrono::duration<Real, std::ratio<86400>>;
  Real d = std::chrono::duration_cast<days_d>(tp_utc.time_since_epoch()).count();
  return 2440587.5 + d;
}

inline Real
siderialTime(Real JD_utc)
{
  Real T = (JD_utc - 2451545.0) / 36525.0;
  Real gmst = 280.46061837 + 360.98564736629 * (JD_utc - 2451545.0) + 0.000387933 * T * T -
              (T * T * T) / 38710000.0;
  return wrap2pi(gmst * degrees);
}

// Sun RA/Dec (~0.1°)
inline std::pair<Real, Real>
sunAngles(Real julian_date)
{
  const auto n = julian_date - 2451545.0;
  const auto L = wrap360(280.460 + 0.9856474 * n);
  const auto g = wrap360(357.528 + 0.9856003 * n);
  const auto gr = g * degrees;
  const auto lam = (L + 1.915 * std::sin(gr) + 0.020 * std::sin(2 * gr)) * degrees;
  const auto T = (julian_date - 2451545.0) / 36525.0;
  const auto eps = (23.439291 - 0.0130042 * T) * degrees; // mean obliquity

  const auto x = std::cos(lam);
  const auto y = std::cos(eps) * std::sin(lam);
  const auto z = std::sin(eps) * std::sin(lam);

  const auto ra = wrap2pi(std::atan2(y, x));
  const auto dec = std::asin(clamp(z, -1.0, 1.0));
  return {ra, dec};
}

// Moon RA/Dec (compact; ~0.2–0.3° typical)
inline std::pair<Real, Real>
moonAngles(Real julian_date)
{
  const auto T = (julian_date - 2451545.0) / 36525.0;
  const auto Lp = wrap360(218.3164477 + 481267.88123421 * T - 0.0015786 * T * T +
                          T * T * T / 538841.0 - T * T * T * T / 65194000.0);
  const auto D = wrap360(297.8501921 + 445267.1114034 * T - 0.0018819 * T * T +
                         T * T * T / 545868.0 - T * T * T * T / 113065000.0);
  const auto M =
      wrap360(357.5291092 + 35999.0502909 * T - 0.0001536 * T * T + T * T * T / 24490000.0);
  const auto Mp = wrap360(134.9633964 + 477198.8675055 * T + 0.0087414 * T * T +
                          T * T * T / 69699.0 - T * T * T * T / 14712000.0);
  const auto F = wrap360(93.2720950 + 483202.0175233 * T - 0.0036539 * T * T -
                         T * T * T / 3526000.0 + T * T * T * T / 863310000.0);

  const auto Dr = D * degrees, Mr = M * degrees, Mpr = Mp * degrees, Fr = F * degrees;
  const auto E = 1.0 - 0.002516 * T - 0.0000074 * T * T;

  const auto lam = Lp + 6.289 * std::sin(Mpr) + 1.274 * std::sin(2 * Dr - Mpr) +
                   0.658 * std::sin(2 * Dr) + 0.214 * std::sin(2 * Mpr) + 0.110 * std::sin(Dr) +
                   0.059 * std::sin(2 * Dr - 2 * Mpr) + 0.057 * std::sin(2 * Dr - Mr - Mpr) * E +
                   0.053 * std::sin(2 * Dr + Mpr) + 0.046 * std::sin(2 * Dr - Mr) * E +
                   0.041 * std::sin(Mr) * E + 0.035 * std::sin(Dr + Mpr) -
                   0.030 * std::sin(Mr + Mpr) * E - 0.015 * std::sin(2 * Dr - 2 * Fr) +
                   0.011 * std::sin(Dr - Mpr);

  const auto bet = 5.128 * std::sin(Fr) + 0.280 * std::sin(Mpr + Fr) + 0.277 * std::sin(Mpr - Fr) +
                   0.173 * std::sin(2 * Dr - Fr) + 0.055 * std::sin(2 * Dr + Fr - Mpr) +
                   0.046 * std::sin(2 * Dr - Fr - Mpr) + 0.033 * std::sin(2 * Dr + Fr) +
                   0.017 * std::sin(2 * Mpr + Fr) + 0.009 * std::sin(2 * Dr + Mpr - Fr) +
                   0.009 * std::sin(2 * Dr - Mpr - Fr);

  const auto eps = (23.439291 - 0.0130042 * T) * degrees;
  const auto lamr = wrap2pi(lam * degrees);
  const auto betr = bet * degrees;

  const auto cb = std::cos(betr), sb = std::sin(betr);
  const auto cl = std::cos(lamr), sl = std::sin(lamr);
  const auto x_ecl = cb * cl;
  const auto y_ecl = cb * sl;
  const auto z_ecl = sb;
  const auto x_eq = x_ecl;
  const auto y_eq = y_ecl * std::cos(eps) - z_ecl * std::sin(eps);
  const auto z_eq = y_ecl * std::sin(eps) + z_ecl * std::cos(eps);

  const auto ra = wrap2pi(std::atan2(y_eq, x_eq));
  const auto dec = std::asin(clamp(z_eq, -1.0, 1.0));
  return {ra, dec};
}

// ---------- ECI(ra,dec) → Earth-fixed (Daniel-frame) ----------
inline RealVectorValue
anglesToVector(Real ra, Real dec, Real gmst)
{
  const auto cd = std::cos(dec), sd = std::sin(dec);
  const auto ca = std::cos(ra), sa = std::sin(ra);
  const auto ux = cd * ca, uy = cd * sa, uz = sd;
  const auto c = std::cos(gmst), s = std::sin(gmst);
  const auto Xstd = c * ux - s * uy;
  const auto Ystd = s * ux + c * uy;
  const auto Zstd = uz;
  RealVectorValue v(-Ystd, Xstd, Zstd);
  v /= v.norm();
  return v;
}

template <class Dur>
inline RealVectorValue
sunVector(std::chrono::time_point<clock_t, Dur> tp_utc)
{
  const auto t = std::chrono::time_point_cast<nanos>(tp_utc);
  const auto julian_date = julianDateFromSystemTime(t);
  const auto [ra, dec] = sunAngles(julian_date);
  return anglesToVector(ra, dec, siderialTime(julian_date));
}

template <class Dur>
inline RealVectorValue
moonVector(std::chrono::time_point<clock_t, Dur> tp_utc)
{
  const auto t = std::chrono::time_point_cast<nanos>(tp_utc);
  const auto julian_date = julianDateFromSystemTime(t);
  const auto [ra, dec] = moonAngles(julian_date);
  return anglesToVector(ra, dec, siderialTime(julian_date));
}

} // namespace astro
} // namespace Moose

    registerMooseObject("ShallowWaterApp", TidalGravityAux);

InputParameters
TidalGravityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Scalar gravity field with simple Sun/Moon tidal correction projected onto local vertical.");

  params.addParam<Real>("g0", 9.80665, "Base gravitational magnitude [m/s^2]");
  params.addParam<bool>("enable_tides", true, "Enable Sun/Moon tidal correction");
  params.addParam<std::time_t>("simulation_start_epoch",
                        0.0,
                        "Absolute UTC time (s since 1970-01-01) corresponding to simulation t=0.");
  params.addParam<std::string>(
      "simulation_start_datetime", "", "Start date/time for t=0 in %Y-%m-%d %H:%M:%S format.");
  params.addParam<Real>("simulation_timezone",
                        0.0,
                        "Offset from UTC in hours (allows `simulation_start_datetime` to be "
                        "specified in local time).");
  params.addParam<Real>("earth_radius", 6.371e6, "Earth mean radius [m]");
  params.addParam<Real>("mu_sun", 1.32712440018e20, "GM_sun [m^3/s^2]");
  params.addParam<Real>("sun_distance", 1.495978707e11, "Mean Sun-Earth distance [m]");
  params.addParam<Real>("mu_moon", 4.9048695e12, "GM_moon [m^3/s^2]");
  params.addParam<Real>("moon_distance", 3.844e8, "Mean Moon-Earth distance [m]");
  MooseEnum sun_dist_model("mean seasonal", "mean");
  params.addParam<MooseEnum>("sun_distance_model",
                             sun_dist_model,
                             "Model for Sun-Earth distance: 'mean' (use sun_distance) or "
                             "'seasonal' (1/R^3 seasonal variation from mean anomaly)");
  return params;
}

TidalGravityAux::TidalGravityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _g0(getParam<Real>("g0")),
    _enable_tides(getParam<bool>("enable_tides")),
    _earth_radius(getParam<Real>("earth_radius")),
    _t0_epoch(getParam<Real>("simulation_start_epoch")),
    _mu_sun(getParam<Real>("mu_sun")),
    _sun_distance(getParam<Real>("sun_distance")),
    _mu_moon(getParam<Real>("mu_moon")),
    _moon_distance(getParam<Real>("moon_distance")),
    _sun_distance_seasonal(getParam<MooseEnum>("sun_distance_model") == "seasonal"),
    _cached_t_step(std::numeric_limits<unsigned int>::max()),
    _a_tide(0.0, 0.0, 0.0)
{
  if (isParamValid("simulation_start_epoch") == isParamValid("simulation_start_datetime"))
    mooseError("Specify either `simulation_start_epoch` or `simulation_start_datetime`.");

  if (isParamValid("simulation_start_datetime"))
    _t0_epoch = static_cast<Real>(parseStartDatetime());
  if (isParamValid("simulation_start_epoch"))
  {
    if (getParam<Real>("simulation_timezone") != 0.0)
      paramError("simulation_timezone",
                 "simulation_start_epoch is always in UTC. `simulation_timezone` is not applied "
                 "and must be zero.");
    _t0_epoch = getParam<std::time_t>("simulation_start_epoch");
  }
}

void
TidalGravityAux::timestepSetup()
{
  const std::time_t t_abs = _t0_epoch + static_cast<std::time_t>(_t);

  // Get Earth-fixed unit vectors toward Sun and Moon at this absolute time
  RealVectorValue s_hat(0, 0, 0), m_hat(0, 0, 0);
  computeSunMoonDirsAtEpoch(t_abs, s_hat, m_hat);

  // Determine Sun-Earth distance model
  Real r_sun = _sun_distance;
  if (_sun_distance_seasonal)
  {
    const Real JD = 2440587.5 + t_abs / 86400.0;
    const Real n = JD - 2451545.0;
    const Real M = (357.529 + 0.9856003 * n) * (libMesh::pi / 180.0); // mean anomaly [rad]
    const Real R_AU = 1.00014 - 0.01671 * std::cos(M) - 0.00014 * std::cos(2.0 * M);
    const Real AU = 1.495978707e11;
    r_sun = R_AU * AU;
  }
  const Real r_moon = _moon_distance;
  const Real a_sun = 2.0 * _mu_sun * _earth_radius / (r_sun * r_sun * r_sun);
  const Real a_moon = 2.0 * _mu_moon * _earth_radius / (r_moon * r_moon * r_moon);

  _a_tide = (_enable_tides ? (a_sun * s_hat + a_moon * m_hat) : RealVectorValue(0, 0, 0));
}

Real
TidalGravityAux::computeValue()
{
  const Real rmag = _q_point[_qp].norm();
  RealVectorValue n_r(0, 0, 1);
  if (rmag > 0.0)
    n_r = _q_point[_qp] / rmag;
  const auto gvec = (-_g0) * n_r + _a_tide;
  return -gvec * n_r;
}

std::time_t
TidalGravityAux::parseStartDatetime() const
{
  const std::string & datetime = getParam<std::string>("simulation_start_datetime");
  const auto offset_hours = getParam<Real>("simulation_timezone");

  try
  {
    return Moose::astro::toUnixEpoch(datetime, offset_hours);
  }
  catch (std::runtime_error & e)
  {
    paramError("simulation_start_datetime", e.what());
  }
}

// Integrate ephemerides utilities for directions
void
TidalGravityAux::computeSunMoonDirsAtEpoch(std::time_t epoch_seconds,
                                           RealVectorValue & sunVector,
                                           RealVectorValue & moonVector) const
{
  using namespace std::chrono;
  const auto tp = Moose::astro::SysTimeNs(seconds(epoch_seconds));

  sunVector = Moose::astro::sunVector(tp);
  moonVector = Moose::astro::moonVector(tp);
}
