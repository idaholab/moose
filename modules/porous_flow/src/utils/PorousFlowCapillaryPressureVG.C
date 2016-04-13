/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureVG.h"

Real
PorousFlowCapillaryPressureVG::effectiveSaturation(Real s, Real sat_r, Real sat_s)
{
  return (s - sat_r)/(sat_s - sat_r);
}

Real
PorousFlowCapillaryPressureVG::capillaryPressure(Real s, Real m, Real sat_r, Real sat_s, Real p0, Real pc_max)
{
  Real sat_eff = PorousFlowCapillaryPressureVG::effectiveSaturation(s, sat_r, sat_s);
  Real pc = 0.0;
  if (0.0 < sat_eff < 1.0)
  {
    Real a = std::pow(sat_eff, - 1.0 / m) - 1.0;
    if (a > 0.0)
      pc = p0 * std::pow(a, 1.0 - m);
  }

  /// Return the max of this and pc_max
  return std::max(pc, pc_max);
}

Real
PorousFlowCapillaryPressureVG::dCapillaryPressure(Real s, Real m, Real sat_r, Real sat_s, Real p0)
{
  Real sat_eff = PorousFlowCapillaryPressureVG::effectiveSaturation(s, sat_r, sat_s);
  Real dpc = 0.0;

  if (0.0 < sat_eff < 1.0)
  {
    Real a = std::pow(sat_eff, - 1.0 / m) - 1.0;
    if (a > 0.0)
      dpc = p0 * (1.0 - m) * std::pow(a, - m) * std::pow(sat_eff, - 1.0 - 1.0 / m) / (m * (sat_s - sat_r));
  }

  return dpc;
}

Real
PorousFlowCapillaryPressureVG::d2CapillaryPressure(Real s, Real m, Real sat_r, Real sat_s, Real p0)
{
  Real d2pc = 0.0;
  Real sat_eff = PorousFlowCapillaryPressureVG::effectiveSaturation(s, sat_r, sat_s);

  if (0.0 < sat_eff < 1.0)
  {
    Real a = std::pow(sat_eff, - 1.0 / m) - 1.0;
    if (a > 0.0)
    {
      d2pc = std::pow(a, - 1.0 - m) * std::pow(sat_eff, - 2.0 - 2.0 / m) - ((1.0 + m) / m) * std::pow(a, - m) *
        std::pow(sat_eff, - 1.0 / m - 2.0);
      d2pc *= p0 * (1.0 - m) / m / (sat_s - sat_r) / (sat_s - sat_r);
    }
  }

  return d2pc;
}
