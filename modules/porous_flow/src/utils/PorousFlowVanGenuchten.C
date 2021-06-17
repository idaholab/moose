//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowVanGenuchten.h"
#include "PorousFlowCubic.h"
#include "libmesh/utility.h"

namespace PorousFlowVanGenuchten
{
Real
effectiveSaturation(Real p, Real alpha, Real m)
{
  Real n, seff;

  if (p >= 0.0)
    return 1.0;
  else
  {
    n = 1.0 / (1.0 - m);
    seff = 1.0 + std::pow(-alpha * p, n);
    return std::pow(seff, -m);
  }
}

Real
dEffectiveSaturation(Real p, Real alpha, Real m)
{
  if (p >= 0.0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - m);
    Real inner = 1.0 + std::pow(-alpha * p, n);
    Real dinner_dp = -n * alpha * std::pow(-alpha * p, n - 1.0);
    Real dseff_dp = -m * std::pow(inner, -m - 1) * dinner_dp;
    return dseff_dp;
  }
}

Real
d2EffectiveSaturation(Real p, Real alpha, Real m)
{
  if (p >= 0.0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - m);
    Real inner = 1.0 + std::pow(-alpha * p, n);
    Real dinner_dp = -n * alpha * std::pow(-alpha * p, n - 1.0);
    Real d2inner_dp2 = n * (n - 1.0) * alpha * alpha * std::pow(-alpha * p, n - 2.0);
    Real d2seff_dp2 = m * (m + 1.0) * std::pow(inner, -m - 2.0) * std::pow(dinner_dp, 2.0) -
                      m * std::pow(inner, -m - 1.0) * d2inner_dp2;
    return d2seff_dp2;
  }
}

Real
capillaryPressure(Real seff, Real alpha, Real m, Real pc_max)
{
  if (seff >= 1.0)
    return 0.0;
  else if (seff <= 0.0)
    return pc_max;
  else
  {
    Real a = std::pow(seff, -1.0 / m) - 1.0;
    return std::min(std::pow(a, 1.0 - m) / alpha, pc_max);
  }
}

Real
dCapillaryPressure(Real seff, Real alpha, Real m, Real pc_max)
{
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;
  else
  {
    Real a = std::pow(seff, -1.0 / m) - 1.0;
    // Return 0 if pc > pc_max
    if (std::pow(a, 1.0 - m) / alpha > pc_max)
      return 0.0;
    else
      return (m - 1.0) * std::pow(a, -m) * std::pow(seff, -1.0 - 1.0 / m) / m / alpha;
  }
}

Real
d2CapillaryPressure(Real seff, Real alpha, Real m, Real pc_max)
{
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;
  else
  {
    Real a = std::pow(seff, -1.0 / m) - 1.0;
    // Return 0 if pc > pc_max
    if (std::pow(a, 1.0 - m) / alpha > pc_max)
      return 0.0;
    else
    {
      Real d2pc = -std::pow(a, -1.0 - m) * std::pow(seff, -2.0 - 2.0 / m) +
                  ((1.0 + m) / m) * std::pow(a, -m) * std::pow(seff, -1.0 / m - 2.0);
      d2pc *= (1.0 - m) / m / alpha;
      return d2pc;
    }
  }
}

Real
relativePermeability(Real seff, Real m)
{
  if (seff <= 0.0)
    return 0.0;
  else if (seff >= 1.0)
    return 1.0;

  const Real a = 1.0 - std::pow(seff, 1.0 / m);
  const Real b = 1.0 - std::pow(a, m);

  return std::sqrt(seff) * Utility::pow<2>(b);
}

Real
dRelativePermeability(Real seff, Real m)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;

  const Real a = 1.0 - std::pow(seff, 1.0 / m);
  const Real da = -1.0 / m * std::pow(seff, 1.0 / m - 1.0);
  const Real b = 1.0 - std::pow(a, m);
  const Real db = -m * std::pow(a, m - 1.0) * da;

  return 0.5 * std::pow(seff, -0.5) * Utility::pow<2>(b) + 2.0 * std::sqrt(seff) * b * db;
}

Real
d2RelativePermeability(Real seff, Real m)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;

  const Real a = 1.0 - std::pow(seff, 1.0 / m);
  const Real da = -1.0 / m * std::pow(seff, 1.0 / m - 1.0);
  const Real d2a = -(1.0 / m) * (1.0 / m - 1.0) * std::pow(seff, 1.0 / m - 2.0);
  const Real b = 1.0 - std::pow(a, m);
  const Real db = -m * std::pow(a, m - 1.0) * da;
  const Real d2b = -m * (m - 1.0) * std::pow(a, m - 2.0) * da * da - m * std::pow(a, m - 1.0) * d2a;

  return -0.25 * std::pow(seff, -1.5) * Utility::pow<2>(b) + 2.0 * std::pow(seff, -0.5) * b * db +
         2.0 * std::sqrt(seff) * db * db + 2.0 * std::sqrt(seff) * b * d2b;
}

Real
relativePermeabilityNW(Real seff, Real m)
{
  if (seff <= 0.0)
    return 0.0;
  else if (seff >= 1.0)
    return 1.0;

  const Real a = std::pow(1.0 - seff, 1.0 / m);
  const Real b = std::pow(1.0 - a, 2.0 * m);

  return std::sqrt(seff) * b;
}

Real
dRelativePermeabilityNW(Real seff, Real m)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;

  const Real a = std::pow(1.0 - seff, 1.0 / m);
  const Real da = -1.0 / m * a / (1.0 - seff);
  const Real b = std::pow(1.0 - a, 2.0 * m);
  const Real db = -2.0 * m * b / (1.0 - a) * da;

  return 0.5 * std::pow(seff, -0.5) * b + std::sqrt(seff) * db;
}

Real
d2RelativePermeabilityNW(Real seff, Real m)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;

  const Real a = std::pow(1.0 - seff, 1.0 / m);
  const Real da = -1.0 / m * a / (1.0 - seff);
  const Real d2a = 1.0 / m * (1.0 / m - 1) * std::pow(1.0 - seff, 1.0 / m - 2.0);
  const Real b = std::pow(1.0 - a, 2.0 * m);
  const Real db = -2.0 * m * b / (1.0 - a) * da;
  const Real d2b =
      -2.0 * m * (db / (1.0 - a) * da + b * Utility::pow<2>(da / (1.0 - a)) + b / (1.0 - a) * d2a);

  return -0.25 * std::pow(seff, -1.5) * b + std::pow(seff, -0.5) * db + std::sqrt(seff) * d2b;
}

Real
capillaryPressureHys(Real sl,
                     Real slmin,
                     Real sgrdel,
                     Real alpha,
                     Real n,
                     const LowCapillaryPressureExtension & low_ext,
                     const HighCapillaryPressureExtension & high_ext)
{
  Real pc = 0.0;
  if (sl < low_ext.S) // important for initializing low_ext that this is < and not <=
  {
    switch (low_ext.strategy)
    {
      case LowCapillaryPressureExtension::QUADRATIC:
        pc = low_ext.Pc + low_ext.dPc * 0.5 * (sl * sl - low_ext.S * low_ext.S) / low_ext.S;
        break;
      case LowCapillaryPressureExtension::EXPONENTIAL:
        pc = low_ext.Pc * std::exp(low_ext.dPc * (sl - low_ext.S) / low_ext.Pc);
        break;
      default:
        pc = low_ext.Pc;
    }
    return pc;
  }
  if (sl > high_ext.S) // important for initializing high_ext that this is >, not >=
  {
    switch (high_ext.strategy)
    {
      case HighCapillaryPressureExtension::POWER:
      {
        if (sl >= 1.0)
          pc = 0.0;
        else
        {
          const Real expon = -high_ext.dPc / high_ext.Pc * (1.0 - high_ext.S);
          pc = high_ext.Pc * std::pow((1.0 - sl) / (1.0 - high_ext.S), expon);
        }
        break;
      }
      default:
        pc = 0.0;
    }
    return pc;
  }
  const Real seff = (sl - slmin) / (1.0 - sgrdel - slmin);
  if (seff >= 1.0)
    pc = 0.0; // no sensible high extension defined
  else if (seff <= 0.0)
    pc = low_ext.Pc; // no sensible low extension defined
  else
  {
    const Real a = std::pow(seff, n / (1.0 - n)) - 1.0;
    pc = (1.0 / alpha) * std::pow(a, 1.0 / n);
  }
  return pc;
}

Real
dcapillaryPressureHys(Real sl,
                      Real slmin,
                      Real sgrdel,
                      Real alpha,
                      Real n,
                      const LowCapillaryPressureExtension & low_ext,
                      const HighCapillaryPressureExtension & high_ext)
{
  Real dpc = 0.0;
  if (sl < low_ext.S) // important for initializing low_ext that this is < and not <=
  {
    switch (low_ext.strategy)
    {
      case LowCapillaryPressureExtension::QUADRATIC:
        dpc = low_ext.dPc * sl / low_ext.S;
        break;
      case LowCapillaryPressureExtension::EXPONENTIAL:
        dpc = low_ext.dPc * std::exp(low_ext.dPc * (sl - low_ext.S) / low_ext.Pc);
        break;
      default:
        dpc = 0.0;
    }
    return dpc;
  }
  if (sl > high_ext.S) // important for initializing high_ext that this is >, not >=
  {
    switch (high_ext.strategy)
    {
      case HighCapillaryPressureExtension::POWER:
      {
        if (sl >= 1.0)
          dpc = 0.0;
        else
        {
          const Real expon = -high_ext.dPc / high_ext.Pc * (1.0 - high_ext.S);
          dpc = high_ext.dPc * std::pow((1.0 - sl) / (1.0 - high_ext.S), expon - 1.0);
        }
        break;
      }
      default:
        dpc = 0.0;
    }
    return dpc;
  }
  const Real seff = (sl - slmin) / (1.0 - sgrdel - slmin);
  if (seff >= 1.0)
    dpc = 0.0; // no sensible high extension defined
  else if (seff <= 0.0)
    dpc = 0.0; // no sensible low extension defined
  else
  {
    const Real a = std::pow(seff, n / (1.0 - n)) - 1.0;
    const Real dseff = 1.0 / (1.0 - sgrdel - slmin);
    const Real dpc_dseff = (1.0 / alpha / (1.0 - n)) * std::pow(a, 1.0 / n - 1.0) *
                           std::pow(seff, n / (1.0 - n) - 1.0);
    dpc = dpc_dseff * dseff;
  }
  return dpc;
}

Real
d2capillaryPressureHys(Real sl,
                       Real slmin,
                       Real sgrdel,
                       Real alpha,
                       Real n,
                       const LowCapillaryPressureExtension & low_ext,
                       const HighCapillaryPressureExtension & high_ext)
{
  Real d2pc = 0.0;
  if (sl < low_ext.S) // important for initializing low_ext that this is < and not <=
  {
    switch (low_ext.strategy)
    {
      case LowCapillaryPressureExtension::QUADRATIC:
        d2pc = low_ext.dPc / low_ext.S;
        break;
      case LowCapillaryPressureExtension::EXPONENTIAL:
        d2pc = std::pow(low_ext.dPc, 2) / low_ext.Pc *
               std::exp(low_ext.dPc * (sl - low_ext.S) / low_ext.Pc);
        break;
      default:
        d2pc = 0.0;
    }
    return d2pc;
  }
  if (sl > high_ext.S) // important for initializing high_ext that this is >, not >=
  {
    switch (high_ext.strategy)
    {
      case HighCapillaryPressureExtension::POWER:
      {
        if (sl >= 1.0)
          d2pc = 0.0;
        else
        {
          const Real expon = -high_ext.dPc / high_ext.Pc * (1.0 - high_ext.S);
          d2pc = high_ext.dPc * (1.0 - expon) / (1.0 - high_ext.S) *
                 std::pow((1.0 - sl) / (1.0 - high_ext.S), expon - 2.0);
        }
        break;
      }
      default:
        d2pc = 0.0;
    }
    return d2pc;
  }
  const Real seff = (sl - slmin) / (1.0 - sgrdel - slmin);
  if (seff >= 1.0)
    d2pc = 0.0; // no sensible high extension defined
  else if (seff <= 0.0)
    d2pc = 0.0; // no sensible low extension defined
  else
  {
    const Real a = std::pow(seff, n / (1.0 - n)) - 1.0;
    const Real dseff = 1.0 / (1.0 - sgrdel - slmin);
    const Real d2pc_dseff =
        (1.0 / alpha / (1.0 - n)) *
        (std::pow(a, 1.0 / n - 2.0) * std::pow(seff, 2.0 * (n / (1.0 - n) - 1.0)) +
         (n / (1.0 - n) - 1.0) * std::pow(a, 1.0 / n - 1.0) * std::pow(seff, n / (1.0 - n) - 2.0));
    d2pc = d2pc_dseff * dseff * dseff;
  }
  return d2pc;
}

Real
saturationHys(Real pc,
              Real slmin,
              Real sgrdel,
              Real alpha,
              Real n,
              const LowCapillaryPressureExtension & low_ext,
              const HighCapillaryPressureExtension & high_ext)
{
  if (pc <= 0)
    return 1.0;
  Real s = 1.0;
  if (pc > low_ext.Pc) // important for initialization of the low_ext that this is > and not >=
  {
    switch (low_ext.strategy)
    {
      case LowCapillaryPressureExtension::QUADRATIC:
      {
        const Real s2 = low_ext.S * low_ext.S + 2.0 * (pc - low_ext.Pc) * low_ext.S / low_ext.dPc;
        if (s2 <= 0.0) // this occurs when we're trying to find a saturation on the wetting curve
                       // defined by sl = sgrDel at pc = Pcd_Del, if this pc is actually impossible
                       // to achieve on this wetting curve
          s = 0.0;
        else
          s = std::sqrt(s2);
        break;
      }
      case LowCapillaryPressureExtension::EXPONENTIAL:
      {
        const Real ss = low_ext.S + std::log(pc / low_ext.Pc) * low_ext.Pc / low_ext.dPc;
        if (ss <= 0.0) // this occurs when we're trying to find a saturation on the
                       // wetting curve defined by sl = sgrDel at pc = Pcd_Del, if this
                       // pc is actually impossible to achieve on this wetting curve
          s = 0.0;
        else
          s = ss;
        break;
      }
      default:
        s = low_ext.S;
    }
    return s;
  }
  if (pc < high_ext.Pc) // important for initialization of the high_ext that this is < and not <=
  {
    switch (high_ext.strategy)
    {
      case HighCapillaryPressureExtension::POWER:
      {
        const Real expon = -high_ext.dPc / high_ext.Pc * (1.0 - high_ext.S);
        s = 1.0 - std::pow(pc / high_ext.Pc, 1.0 / expon) * (1.0 - high_ext.S);
        break;
      }
      default:
        s = high_ext.S;
    }
    return s;
  }
  if (pc == std::numeric_limits<Real>::max())
    s = 0.0;
  else
  {
    const Real seffpow = 1.0 + std::pow(pc * alpha, n);
    const Real seff = std::pow(seffpow, (1.0 - n) / n);
    s = (1.0 - sgrdel - slmin) * seff + slmin;
  }
  return s;
}

Real
dsaturationHys(Real pc,
               Real slmin,
               Real sgrdel,
               Real alpha,
               Real n,
               const LowCapillaryPressureExtension & low_ext,
               const HighCapillaryPressureExtension & high_ext)
{
  if (pc <= 0)
    return 0.0;
  Real ds = 0.0;
  if (pc > low_ext.Pc) // important for initialization of the low_ext that this is > and not >=
  {
    switch (low_ext.strategy)
    {
      case LowCapillaryPressureExtension::QUADRATIC:
      {
        const Real s2 = low_ext.S * low_ext.S + 2.0 * (pc - low_ext.Pc) * low_ext.S / low_ext.dPc;
        if (s2 <= 0.0) // this occurs when we're trying to find a saturation on the wetting curve
                       // defined by sl = sgrDel at pc = Pcd_Del, if this pc is actually impossible
                       // to achieve on this wetting curve
          ds = 0.0;
        else
        {
          const Real ds2 = 2.0 * low_ext.S / low_ext.dPc;
          ds = 0.5 * ds2 / std::sqrt(s2);
        }
        break;
      }
      case LowCapillaryPressureExtension::EXPONENTIAL:
      {
        const Real s = low_ext.S + std::log(pc / low_ext.Pc) * low_ext.Pc / low_ext.dPc;
        if (s <= 0.0) // this occurs when we're trying to find a saturation on the
                      // wetting curve defined by sl = sgrDel at pc = Pcd_Del, if this
                      // pc is actually impossible to achieve on this wetting curve
          ds = 0.0;
        else
          ds = low_ext.Pc / pc / low_ext.dPc;
        break;
      }
      default:
        ds = 0.0;
    }
    return ds;
  }
  if (pc < high_ext.Pc) // important for initialization of the high_ext that this is < and not <=
  {
    switch (high_ext.strategy)
    {
      case HighCapillaryPressureExtension::POWER:
      {
        const Real expon = -high_ext.dPc / high_ext.Pc * (1.0 - high_ext.S);
        ds = -(1.0 - high_ext.S) / pc / expon * std::pow(pc / high_ext.Pc, 1.0 / expon);
        break;
      }
      default:
        ds = 0.0;
    }
    return ds;
  }
  if (pc == std::numeric_limits<Real>::max())
    ds = 0.0;
  else
  {
    const Real seffpow = 1.0 + std::pow(pc * alpha, n);
    const Real dseffpow = n * (seffpow - 1.0) / pc;
    const Real seff = std::pow(seffpow, (1.0 - n) / n);
    const Real dseff = (1.0 - n) / n * seff / seffpow * dseffpow;
    ds = (1.0 - sgrdel - slmin) * dseff;
  }
  return ds;
}

Real
d2saturationHys(Real pc,
                Real slmin,
                Real sgrdel,
                Real alpha,
                Real n,
                const LowCapillaryPressureExtension & low_ext,
                const HighCapillaryPressureExtension & high_ext)
{
  if (pc <= 0)
    return 0.0;
  Real d2s = 0.0;
  if (pc > low_ext.Pc) // important for initialization of the low_ext that this is > and not >=
  {
    switch (low_ext.strategy)
    {
      case LowCapillaryPressureExtension::QUADRATIC:
      {
        const Real s2 = low_ext.S * low_ext.S + 2.0 * (pc - low_ext.Pc) * low_ext.S / low_ext.dPc;
        if (s2 <= 0.0) // this occurs when we're trying to find a saturation on the wetting curve
                       // defined by sl = sgrDel at pc = Pcd_Del, if this pc is actually impossible
                       // to achieve on this wetting curve
          d2s = 0.0;
        else
        {
          const Real ds2 = 2.0 * low_ext.S / low_ext.dPc;
          d2s = -0.25 * ds2 * ds2 / std::pow(s2, 1.5);
        }
        break;
      }
      case LowCapillaryPressureExtension::EXPONENTIAL:
      {
        const Real s = low_ext.S + std::log(pc / low_ext.Pc) * low_ext.Pc / low_ext.dPc;
        if (s <= 0.0) // this occurs when we're trying to find a saturation on the
                      // wetting curve defined by sl = sgrDel at pc = Pcd_Del, if this
                      // pc is actually impossible to achieve on this wetting curve
          d2s = 0.0;
        else
          d2s = -low_ext.Pc / std::pow(pc, 2.0) / low_ext.dPc;
        break;
      }
      default:
        d2s = 0.0;
    }
    return d2s;
  }
  if (pc < high_ext.Pc) // important for initialization of the high_ext that this is < and not <=
  {
    switch (high_ext.strategy)
    {
      case HighCapillaryPressureExtension::POWER:
      {
        const Real expon = -high_ext.dPc / high_ext.Pc * (1.0 - high_ext.S);
        d2s = -(1.0 - high_ext.S) * (1.0 / expon) * (1.0 / expon - 1.0) /
              std::pow(high_ext.Pc, 2.0) * std::pow(pc / high_ext.Pc, 1.0 / expon - 2.0);
        break;
      }
      default:
        d2s = 0.0;
    }
    return d2s;
  }
  if (pc == std::numeric_limits<Real>::max())
    d2s = 0.0;
  else
  {
    const Real seffpow = 1.0 + std::pow(pc * alpha, n);
    const Real dseffpow = n * (seffpow - 1.0) / pc;
    const Real d2seffpow = (n - 1.0) * dseffpow / pc;
    const Real seff = std::pow(seffpow, (1.0 - n) / n);
    const Real dseff = (1.0 - n) / n * seff / seffpow * dseffpow;
    const Real d2seff =
        (1.0 - n) / n *
        (dseff * dseffpow - seff * dseffpow * dseffpow / seffpow + seff * d2seffpow) / seffpow;
    d2s = (1.0 - sgrdel - slmin) * d2seff;
  }
  return d2s;
}

Real
relativePermeabilityHys(Real sl,
                        Real slr,
                        Real sgrdel,
                        Real sgrmax,
                        Real sldel,
                        Real m,
                        Real upper_liquid_param,
                        Real y0,
                        Real y0p,
                        Real y1,
                        Real y1p)
{
  if (sl <= slr) // by the definition of slr, always return 0
    return 0.0;
  const Real sl_bar = (sl - slr) / (1.0 - slr); // effective saturation
  // a and b are useful parameters.  Define b along the drying curve initially, and
  // modify a and b appropriately if the wetting result is required
  Real a = 0;
  Real b = 0;
  if (sgrdel == 0.0 || sl <= sldel) // along the drying curve
    a = std::pow(1.0 - std::pow(sl_bar, 1.0 / m), m);
  else // along the wetting curve
  {
    // In most cases, use sldel and sgrdel as provided to this function.  However, because "there is
    // no hysteresis along the extension" according to p6 of Doughty2008, if the turning point is
    // less than slr, then use the expressions for the case when the turning point was slr
    const Real my_sldel = (sldel < slr) ? slr : sldel;
    const Real my_sgrdel = (sldel < slr) ? sgrmax : sgrdel;
    if (sl >= 1.0 - 0.5 * my_sgrdel)
    {
      // follow the drying curve.  The parameter b has already been defined.  It
      // is important for initialization of the curic that the above condition is >= and not >
      a = std::pow(1.0 - std::pow(sl_bar, 1.0 / m), m);
    }
    else if (sl > upper_liquid_param * (1.0 - my_sgrdel))
    {
      // follow the cubic modification of the wetting curve.  Immediately exit from this function by
      // returning the cubic result
      return PorousFlowCubic::cubic(
          sl, upper_liquid_param * (1.0 - my_sgrdel), y0, y0p, 1.0 - 0.5 * my_sgrdel, y1, y1p);
    }
    else
    {
      // standard case of wetting curve outside the cubic-modification and drying-curve regions
      const Real sl_bar_del = (my_sldel - slr) / (1.0 - slr);
      const Real s_gt_bar =
          my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel);
      a = (1 - s_gt_bar / (1.0 - sl_bar_del)) *
          std::pow(1.0 - std::pow(sl_bar + s_gt_bar, 1.0 / m), m);
      b = s_gt_bar / (1.0 - sl_bar_del) * std::pow(1.0 - std::pow(sl_bar_del, 1.0 / m), m);
    }
  }
  return std::sqrt(sl_bar) * Utility::pow<2>(1.0 - a - b);
}

Real
drelativePermeabilityHys(Real sl,
                         Real slr,
                         Real sgrdel,
                         Real sgrmax,
                         Real sldel,
                         Real m,
                         Real upper_liquid_param,
                         Real y0,
                         Real y0p,
                         Real y1,
                         Real y1p)
{
  if (sl <= slr) // by the definition of slr, always return 0
    return 0.0;
  if (sl == 1.0) // derivative is infinite at this point
    return std::numeric_limits<Real>::max();
  const Real sl_bar = (sl - slr) / (1.0 - slr); // effective saturation
  const Real sl_bar_prime = 1.0 / (1.0 - slr);
  // a and b are useful parameters.  Define b along the drying curve initially, and
  // modify a and b appropriately if the wetting result is required
  Real a = 0;
  Real a_prime = 0.0;
  Real b = 0;
  Real b_prime = 0.0;
  if (sgrdel == 0.0 || sl <= sldel) // along the drying curve
  {
    const Real c = std::pow(sl_bar, 1.0 / m);
    const Real dc_dsbar = c / m / sl_bar;
    a = std::pow(1.0 - c, m);
    const Real da_dsbar = -m * a / (1.0 - c) * dc_dsbar;
    a_prime = da_dsbar * sl_bar_prime;
  }
  else // along the wetting curve
  {
    // In most cases, use sldel and sgrdel as provided to this function.  However, because "there is
    // no hysteresis along the extension" according to p6 of Doughty2008, if the turning point is
    // less than slr, then use the expressions for the case when the turning point was slr
    const Real my_sldel = (sldel < slr) ? slr : sldel;
    const Real my_sgrdel = (sldel < slr) ? sgrmax : sgrdel;
    if (sl >= 1.0 - 0.5 * my_sgrdel)
    {
      // follow the drying curve.  The parameter b has already been defined.  It
      // is important for initialization of the curic that the above condition is >= and not >
      const Real c = std::pow(sl_bar, 1.0 / m);
      const Real dc_dsbar = c / m / sl_bar;
      a = std::pow(1.0 - c, m);
      const Real da_dsbar = -m * a / (1.0 - c) * dc_dsbar;
      a_prime = da_dsbar * sl_bar_prime;
    }
    else if (sl > upper_liquid_param * (1.0 - my_sgrdel))
    {
      // follow the cubic modification of the wetting curve.  Immediately exit from this function by
      // returning the cubic result
      return PorousFlowCubic::dcubic(
          sl, upper_liquid_param * (1.0 - my_sgrdel), y0, y0p, 1.0 - 0.5 * my_sgrdel, y1, y1p);
    }
    else
    {
      // standard case of wetting curve outside the cubic-modification and drying-curve regions
      const Real sl_bar_del = (my_sldel - slr) / (1.0 - slr);
      const Real s_gt_bar =
          my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel);
      const Real s_gt_bar_prime = my_sgrdel / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel);
      const Real c = std::pow(sl_bar + s_gt_bar, 1.0 / m);
      const Real c_prime = c / m / (sl_bar + s_gt_bar) * (sl_bar_prime + s_gt_bar_prime);
      a = (1 - s_gt_bar / (1.0 - sl_bar_del)) * std::pow(1.0 - c, m);
      a_prime =
          -s_gt_bar_prime / (1.0 - sl_bar_del) * std::pow(1.0 - c, m) - m * a / (1.0 - c) * c_prime;
      b = s_gt_bar / (1.0 - sl_bar_del) * std::pow(1.0 - std::pow(sl_bar_del, 1.0 / m), m);
      b_prime = s_gt_bar_prime * b / s_gt_bar;
    }
  }
  const Real kr = std::sqrt(sl_bar) * Utility::pow<2>(1.0 - a - b);
  return 0.5 * kr / sl_bar * sl_bar_prime -
         std::sqrt(sl_bar) * 2.0 * (1.0 - a - b) * (a_prime + b_prime);
}

Real
relativePermeabilityNWHys(Real sl,
                          Real slr,
                          Real sgrdel,
                          Real sgrmax,
                          Real sldel,
                          Real m,
                          Real gamma,
                          Real k_rg_max,
                          Real y0p)
{
  if (sl < slr)
  {
    // in the extended region, so immediately return with the relevant value
    if (k_rg_max == 1.0)
      return 1.0;
    return PorousFlowCubic::cubic(sl, 0.0, 1.0, 0.0, slr, k_rg_max, y0p);
  }
  if (sl > 1.0 - sgrdel) // saturation is above 1.0 - residual gas saturation
    return 0.0;
  const Real sl_bar = (sl - slr) / (1.0 - slr);
  Real s_gt_bar = 0.0; // initialize this parameter as if on the drying curve
  if (sgrdel != 0.0 && sl > sldel)
  {
    // On the wetting curve
    // In most cases, use sldel and sgrdel as provided to this function.  However, because "there is
    // no hysteresis along the extension" according to p6 of Doughty2008, if the turning point is
    // less than slr, then use the expressions for the case when the turning point was slr
    const Real my_sldel = (sldel < slr) ? slr : sldel;
    const Real my_sgrdel = (sldel < slr) ? sgrmax : sgrdel;
    s_gt_bar = my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel);
  }
  Real kr = 0.0;
  if (sl_bar + s_gt_bar < 1.0) // check for the condition where sl is too big, in which case kr =
                               // 0, irrespective of hysteresis
  {
    const Real a = std::pow(1.0 - (sl_bar + s_gt_bar), gamma);
    const Real c = std::pow(sl_bar + s_gt_bar, 1.0 / m);
    const Real b = std::pow(1.0 - c, 2.0 * m);
    kr = k_rg_max * a * b;
  }
  return kr;
}

Real
drelativePermeabilityNWHys(Real sl,
                           Real slr,
                           Real sgrdel,
                           Real sgrmax,
                           Real sldel,
                           Real m,
                           Real gamma,
                           Real k_rg_max,
                           Real y0p)
{
  if (sl < slr)
  {
    // in the extended region, so immediately return with the relevant value
    if (k_rg_max == 1.0)
      return 0.0;
    return PorousFlowCubic::dcubic(sl, 0.0, 1.0, 0.0, slr, k_rg_max, y0p);
  }
  if (sl > 1.0 - sgrdel) // saturation is above 1.0 - residual gas saturation
    return 0.0;
  const Real sl_bar = (sl - slr) / (1.0 - slr);
  const Real sl_bar_prime = 1.0 / (1.0 - slr);
  Real s_gt_bar = 0.0;       // initialize this parameter as if on the drying curve
  Real s_gt_bar_prime = 0.0; // again, assume on drying curve
  if (sgrdel != 0.0 && sl > sldel)
  {
    // On the wetting curve
    // In most cases, use sldel and sgrdel as provided to this function.  However, because "there is
    // no hysteresis along the extension" according to p6 of Doughty2008, if the turning point is
    // less than slr, then use the expressions for the case when the turning point was slr
    const Real my_sldel = (sldel < slr) ? slr : sldel;
    const Real my_sgrdel = (sldel < slr) ? sgrmax : sgrdel;
    s_gt_bar = my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel);
    s_gt_bar_prime = my_sgrdel / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel);
  }
  Real kr_prime = 0.0;
  if (sl_bar + s_gt_bar < 1.0) // check for the condition where sl is too big, in which case kr =
                               // 0, irrespective of hysteresis
  {
    const Real a = std::pow(1.0 - (sl_bar + s_gt_bar), gamma);
    const Real a_prime = -gamma * a / (1.0 - (sl_bar + s_gt_bar)) * (sl_bar_prime + s_gt_bar_prime);
    const Real c = std::pow(sl_bar + s_gt_bar, 1.0 / m);
    const Real c_prime =
        (c == 0 ? 0.0 : c / m / (sl_bar + s_gt_bar) * (sl_bar_prime + s_gt_bar_prime));
    const Real b = std::pow(1.0 - c, 2.0 * m);
    const Real b_prime = -2.0 * m * b / (1.0 - c) * c_prime;
    kr_prime = k_rg_max * (a * b_prime + a_prime * b);
  }
  return kr_prime;
}
}
