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
    _start_datetime(getParam<std::string>("start_datetime")),
    _mu_sun(getParam<Real>("mu_sun")),
    _sun_distance(getParam<Real>("sun_distance")),
    _sun_period(getParam<Real>("sun_period")),
    _mu_moon(getParam<Real>("mu_moon")),
    _moon_distance(getParam<Real>("moon_distance")),
    _moon_period(getParam<Real>("moon_period")),
    _sun_distance_seasonal(getParam<MooseEnum>("sun_distance_model") == MooseEnum("seasonal")),
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

// moved below after astro namespace where robust parser is defined

void
TidalGravityAux::updateTidal()
{
  const Real t_abs = _t0_epoch + _t; // seconds UTC

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

// Utility functions that need to be integrated properly:

// astro_dirs_cxx17.hpp  (C++17)
// - Manual date/time parser → std::chrono::system_clock::time_point (UTC)
// - Sun (~0.1°) and Moon (~0.2–0.3°) direction vectors
// - Output frame: x=-Ystd, y=+Xstd, z=Zstd (North pole = (0,0,1); Greenwich/equator = (0,1,0))
#pragma once
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace astro {

using clock_t   = std::chrono::system_clock;
using nanos     = std::chrono::nanoseconds;
using SysTimeNs = std::chrono::time_point<clock_t, nanos>;

struct Vec3 { double x,y,z; };

// ---------- math utils ----------
constexpr double PI  = 3.141592653589793238462643383279502884;
constexpr double TAU = 2.0 * PI;
constexpr double DEG = PI/180.0;

inline double wrap2pi(double a){ a = std::fmod(a, TAU); return a < 0 ? a + TAU : a; }
inline double wrap360(double d){ d = std::fmod(d, 360.0); return d < 0 ? d + 360.0 : d; }
inline double clamp(double x, double lo, double hi){ return x<lo?lo:(x>hi?hi:x); }

// ---------- Julian Date conversions ----------
inline double jd_from_sys_time(const SysTimeNs& tp_utc) {
    // JD = 2440587.5 + seconds_since_unix_epoch/86400
    using days_d = std::chrono::duration<double, std::ratio<86400>>;
    double d = std::chrono::duration_cast<days_d>(tp_utc.time_since_epoch()).count();
    return 2440587.5 + d;
}

// ---------- Sidereal time (GMST) ----------
inline double gmst_rad(double JD_utc){
    double T = (JD_utc - 2451545.0)/36525.0;
    double gmst = 280.46061837
                + 360.98564736629*(JD_utc - 2451545.0)
                + 0.000387933*T*T
                - (T*T*T)/38710000.0;
    return wrap2pi(gmst * DEG);
}

// ---------- Sun RA/Dec (~0.1°) ----------
inline void sun_radec(double JD, double& ra, double& dec){
    double n   = JD - 2451545.0;
    double L   = wrap360(280.460 + 0.9856474*n);
    double g   = wrap360(357.528 + 0.9856003*n);
    double gr  = g*DEG;
    double lam = (L + 1.915*std::sin(gr) + 0.020*std::sin(2*gr)) * DEG;
    double T   = (JD-2451545.0)/36525.0;
    double eps = (23.439291 - 0.0130042*T) * DEG; // mean obliquity
    double x = std::cos(lam);
    double y = std::cos(eps)*std::sin(lam);
    double z = std::sin(eps)*std::sin(lam);
    ra  = wrap2pi(std::atan2(y, x));
    dec = std::asin(clamp(z,-1.0,1.0));
}

// ---------- Moon RA/Dec (compact; ~0.2–0.3° typical) ----------
inline void moon_radec(double JD, double& ra, double& dec){
    double T = (JD - 2451545.0)/36525.0;
    double Lp = wrap360(218.3164477 + 481267.88123421*T - 0.0015786*T*T + T*T*T/538841.0 - T*T*T*T/65194000.0);
    double D  = wrap360(297.8501921 + 445267.1114034*T  - 0.0018819*T*T + T*T*T/545868.0  - T*T*T*T/113065000.0);
    double M  = wrap360(357.5291092 +   35999.0502909*T  - 0.0001536*T*T + T*T*T/24490000.0);
    double Mp = wrap360(134.9633964 + 477198.8675055*T  + 0.0087414*T*T + T*T*T/69699.0   - T*T*T*T/14712000.0);
    double F  = wrap360( 93.2720950 + 483202.0175233*T  - 0.0036539*T*T - T*T*T/3526000.0 + T*T*T*T/863310000.0);

    double Dr = D*DEG, Mr = M*DEG, Mpr = Mp*DEG, Fr = F*DEG;
    double E  = 1.0 - 0.002516*T - 0.0000074*T*T;

    double lam = Lp
        + 6.289 * std::sin(Mpr)
        + 1.274 * std::sin(2*Dr - Mpr)
        + 0.658 * std::sin(2*Dr)
        + 0.214 * std::sin(2*Mpr)
        + 0.110 * std::sin(Dr)
        + 0.059 * std::sin(2*Dr - 2*Mpr)
        + 0.057 * std::sin(2*Dr - Mr - Mpr) * E
        + 0.053 * std::sin(2*Dr + Mpr)
        + 0.046 * std::sin(2*Dr - Mr) * E
        + 0.041 * std::sin(Mr) * E
        + 0.035 * std::sin(Dr + Mpr)
        - 0.030 * std::sin(Mr + Mpr) * E
        - 0.015 * std::sin(2*Dr - 2*Fr)
        + 0.011 * std::sin(Dr - Mpr);

    double bet = 5.128 * std::sin(Fr)
        + 0.280 * std::sin(Mpr + Fr)
        + 0.277 * std::sin(Mpr - Fr)
        + 0.173 * std::sin(2*Dr - Fr)
        + 0.055 * std::sin(2*Dr + Fr - Mpr)
        + 0.046 * std::sin(2*Dr - Fr - Mpr)
        + 0.033 * std::sin(2*Dr + Fr)
        + 0.017 * std::sin(2*Mpr + Fr)
        + 0.009 * std::sin(2*Dr + Mpr - Fr)
        + 0.009 * std::sin(2*Dr - Mpr - Fr);

    double eps  = (23.439291 - 0.0130042*T) * DEG;
    double lamr = wrap2pi(lam * DEG);
    double betr = bet * DEG;

    double cb = std::cos(betr), sb = std::sin(betr);
    double cl = std::cos(lamr), sl = std::sin(lamr);
    double x_ecl = cb * cl;
    double y_ecl = cb * sl;
    double z_ecl = sb;
    double x_eq = x_ecl;
    double y_eq = y_ecl*std::cos(eps) - z_ecl*std::sin(eps);
    double z_eq = y_ecl*std::sin(eps) + z_ecl*std::cos(eps);

    ra  = wrap2pi(std::atan2(y_eq, x_eq));
    dec = std::asin(clamp(z_eq, -1.0, 1.0));
}

// ---------- ECI(ra,dec) → Earth-fixed (Daniel-frame) ----------
inline Vec3 radec_to_dir(double ra, double dec, double gmst){
    // ECI unit
    double cd = std::cos(dec), sd = std::sin(dec);
    double ca = std::cos(ra),  sa = std::sin(ra);
    double ux = cd*ca, uy = cd*sa, uz = sd;
    // rotate to standard ECEF: Rz(gmst)
    double c = std::cos(gmst), s = std::sin(gmst);
    double Xstd =  c*ux - s*uy;
    double Ystd =  s*ux + c*uy;
    double Zstd =  uz;
    // map to your axes
    Vec3 v{ -Ystd, Xstd, Zstd };
    // normalize (defensive)
    double r = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    v.x/=r; v.y/=r; v.z/=r;
    return v;
}

// ===================== Parsing (C++17, no locale deps) =====================
// Accepts:
//  "YYYY-MM-DD"
//  "YYYY-MM-DD[ T]HH:MM"
//  "YYYY-MM-DD[ T]HH:MM:SS[.frac]"
//  Optional zone: 'Z' or ±HH[:MM] or ±HHMM or ±HH (interpreted as numeric offset)
// Returns a UTC time_point (SysTimeNs). If no zone is present, assumes UTC.
inline SysTimeNs parse_datetime_utc(std::string_view sv) {
    // trim
    auto issp = [](char c){ return std::isspace(static_cast<unsigned char>(c))!=0; };
    size_t a=0,b=sv.size(); while (a<b && issp(sv[a])) ++a; while (b>a && issp(sv[b-1])) --b;
    if (b-a < 10) throw std::runtime_error("datetime too short");
    std::string s(sv.substr(a,b-a));
    // normalize 'T' → ' '
    for(char& c: s) if (c=='T') c=' ';

    auto req_digit = [](char c){ return c>='0' && c<='9'; };
    auto read_int = [&](const std::string& str, size_t& i, int width)->int{
        int v=0; for(int k=0;k<width;++k){ if (i>=str.size() || !req_digit(str[i])) throw std::runtime_error("bad integer"); v = 10*v + (str[i++]-'0'); }
        return v;
    };

    size_t i=0;
    // Date
    int Y = read_int(s,i,4);
    if (i>=s.size() || s[i]!='-') throw std::runtime_error("expected '-' after year");
    ++i;
    int Mo = read_int(s,i,2);
    if (i>=s.size() || s[i]!='-') throw std::runtime_error("expected '-' after month");
    ++i;
    int D = read_int(s,i,2);

    // defaults
    int H=0, Mi=0; double Se=0.0;
    int tz_min = 0; // minutes east of UTC

    // Optional time
    while (i<s.size() && issp(s[i])) ++i;
    if (i<s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
        H  = read_int(s,i,2);
        if (i>=s.size() || s[i]!=':') throw std::runtime_error("expected ':' after hour");
        ++i;
        Mi = read_int(s,i,2);
        // optional :SS[.frac]
        if (i<s.size() && s[i]==':') {
            ++i;
            int sec = read_int(s,i,2);
            Se = static_cast<double>(sec);
            if (i<s.size() && s[i]=='.') {
                ++i;
                // fractional seconds: read up to 9 digits
                double mul = 0.1;
                while (i<s.size() && req_digit(s[i])) {
                    Se += (s[i]-'0')*mul;
                    mul *= 0.1;
                    ++i;
                }
            }
        }
        // optional zone
        // skip spaces
        while (i<s.size() && issp(s[i])) ++i;
        if (i<s.size()) {
            if (s[i]=='Z' || s[i]=='z') {
                ++i; tz_min = 0;
            } else if (s[i]=='+' || s[i]=='-') {
                int sign = (s[i]=='-') ? -1 : 1; ++i;
                // read HH[[:]MM] or HHMM
                int zh=0, zm=0;
                // at least two digits for hour
                if (i+1>=s.size() || !req_digit(s[i]) || !req_digit(s[i+1])) throw std::runtime_error("bad zone hour");
                zh = read_int(s,i,2);
                if (i<s.size() && s[i]==':') { ++i; // HH:MM
                    if (i+1>=s.size() || !req_digit(s[i]) || !req_digit(s[i+1])) throw std::runtime_error("bad zone minute");
                    zm = read_int(s,i,2);
                } else if (i+1<s.size() && req_digit(s[i]) && req_digit(s[i+1])) {
                    // HHMM packed
                    zm = read_int(s,i,2);
                } // else HH only
                tz_min = sign*(zh*60 + zm);
            }
        }
    }

    // after parsing, trailing spaces allowed
    while (i<s.size() && issp(s[i])) ++i;
    if (i != s.size()) throw std::runtime_error("unexpected trailing characters");

    // Convert civil (local at given offset) → JD (UTC) with Meeus algorithm
    int y = Y, m = Mo; if (m <= 2){ y -= 1; m += 12; }
    int A = y/100;
    int B = 2 - A + A/4;
    double day = D + (H + (Mi + Se/60.0)/60.0)/24.0;      // fractional day (local clock)
    long long C = static_cast<long long>(std::floor(365.25*(y + 4716)));
    long long Dm = static_cast<long long>(std::floor(30.6001*(m + 1)));
    double JD_local = C + Dm + day + B - 1524.5;
    // shift to UTC using numeric offset (minutes east of UTC)
    double JD_utc = JD_local - (static_cast<double>(tz_min) / (24.0*60.0));

    // Turn JD_utc into system_clock::time_point (UTC)
    double sec = (JD_utc - 2440587.5) * 86400.0;
    long long s_int = static_cast<long long>(std::floor(sec));
    double frac = sec - static_cast<double>(s_int);
    long long ns = static_cast<long long>(std::llround(frac * 1e9));
    // normalize ns into [0,1e9)
    if (ns >= 1000000000LL) { ns -= 1000000000LL; ++s_int; }
    if (ns < 0)              { ns += 1000000000LL; --s_int; }

    return SysTimeNs(std::chrono::seconds(s_int) + nanos(ns));
}

// ===================== Public API =====================
template<class Dur>
inline Vec3 sun_dir(std::chrono::time_point<clock_t, Dur> tp_utc){
    SysTimeNs t = std::chrono::time_point_cast<nanos>(tp_utc);
    double JD = jd_from_sys_time(t);
    double ra, dec; sun_radec(JD, ra, dec);
    return radec_to_dir(ra, dec, gmst_rad(JD));
}

template<class Dur>
inline Vec3 moon_dir(std::chrono::time_point<clock_t, Dur> tp_utc){
    SysTimeNs t = std::chrono::time_point_cast<nanos>(tp_utc);
    double JD = jd_from_sys_time(t);
    double ra, dec; moon_radec(JD, ra, dec);
    return radec_to_dir(ra, dec, gmst_rad(JD));
}

} // namespace astro

std::time_t
TidalGravityAux::parseStartDatetimeUTC(const std::string & s) const
{
  // Robust parser with time zone support via astro::parse_datetime_utc
  try
  {
    astro::SysTimeNs tp = astro::parse_datetime_utc(s);
    using secs = std::chrono::duration<long long>;
    auto dur = std::chrono::duration_cast<secs>(tp.time_since_epoch());
    return static_cast<std::time_t>(dur.count());
  }
  catch (const std::exception &)
  {
    // Fallback: accept simple UTC formats without zone
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
      throw;
#if defined(_WIN32)
    std::time_t t = _mkgmtime(&tm);
#else
    std::time_t t = timegm(&tm);
#endif
    if (t == static_cast<std::time_t>(-1))
      throw;
    return t;
  }
}

// Integrate ephemerides utilities for directions
void
TidalGravityAux::computeSunMoonDirsAtEpoch(Real epoch_seconds,
                                           RealVectorValue & sun_dir,
                                           RealVectorValue & moon_dir) const
{
  using namespace std::chrono;

  // Convert seconds to system_clock::time_point with nanosecond precision
  long long s_int = static_cast<long long>(std::floor(epoch_seconds));
  double frac = epoch_seconds - static_cast<double>(s_int);
  long long ns = static_cast<long long>(std::llround(frac * 1e9));
  if (ns >= 1000000000LL) { ns -= 1000000000LL; ++s_int; }
  if (ns < 0)              { ns += 1000000000LL; --s_int; }

  astro::SysTimeNs tp = astro::SysTimeNs(seconds(s_int) + astro::nanos(ns));

  const auto s = astro::sun_dir(tp);
  const auto m = astro::moon_dir(tp);

  sun_dir = RealVectorValue(s.x, s.y, s.z);
  moon_dir = RealVectorValue(m.x, m.y, m.z);
}

/* ---------------- Example (C++17) ----------------
#include <iostream>
int main(){
    using namespace std::chrono;

    // Parse once (UTC if no zone; Z or ±HH[:MM] accepted):
    astro::SysTimeNs t0 = astro::parse_datetime_utc("2025-09-07 18:00:00-06:00");

    // Reuse with offsets:
    auto t1 = t0 + minutes(15);
    auto t2 = t0 + hours(2);

    astro::Vec3 s0 = astro::sun_dir(t0);
    astro::Vec3 s1 = astro::sun_dir(t1);
    astro::Vec3 m2 = astro::moon_dir(t2);

    std::cout.setf(std::ios::fixed); std::cout.precision(6);
    std::cout << "Sun(t0): " << s0.x << ", " << s0.y << ", " << s0.z << "\n";
    std::cout << "Sun(t0+15m): " << s1.x << ", " << s1.y << ", " << s1.z << "\n";
    std::cout << "Moon(t0+2h): " << m2.x << ", " << m2.y << ", " << m2.z << "\n";
}
--------------------------------------------------- */
