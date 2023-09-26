//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatConductionNames.h"

namespace HeatTransferModels
{

/**
 * Computes the thermal conductance across a cylindrical medium.
 *
 * Note that thermal conductance has the same units as a heat transfer coefficient.
 *
 * @param[in] k        Thermal conductivity of the medium
 * @param[in] r_inner  Inner radius
 * @param[in] r_outer  Outer radius
 */
template <typename T1, typename T2, typename T3>
auto
cylindricalThermalConductance(const T1 & k, const T2 & r_inner, const T3 & r_outer)
{
  mooseAssert(r_outer > r_inner, "Outer radius must be larger than inner radius.");

  const auto r_avg = 0.5 * (r_inner + r_outer);
  return k / (r_avg * std::log(r_outer / r_inner));
}

/**
 * Computes the conduction heat flux across a cylindrical gap.
 *
 * The convention is that positive heat fluxes correspond to heat moving from
 * the inner surface to the outer surface.
 *
 * @param[in] k_gap    Gap thermal conductivity
 * @param[in] r_inner  Inner radius
 * @param[in] r_outer  Outer radius
 * @param[in] T_inner  Inner temperature
 * @param[in] T_outer  Outer temperature
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5>
auto
cylindricalGapConductionHeatFlux(const T1 & k_gap,
                                 const T2 & r_inner,
                                 const T3 & r_outer,
                                 const T4 & T_inner,
                                 const T5 & T_outer)
{
  return cylindricalThermalConductance(k_gap, r_inner, r_outer) * (T_inner - T_outer);
}

/**
 * Computes the radiation heat flux across a cylindrical gap.
 *
 * The convention is that positive heat fluxes correspond to heat moving from
 * the inner surface to the outer surface.
 *
 * @param[in] r_inner      Inner radius
 * @param[in] r_outer      Outer radius
 * @param[in] emiss_inner  Inner emissivity
 * @param[in] emiss_outer  Outer emissivity
 * @param[in] T_inner      Inner temperature
 * @param[in] T_outer      Outer temperature
 * @param[in] sigma        The Stefan-Boltzmann constant
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
auto
cylindricalGapRadiationHeatFlux(const T1 & r_inner,
                                const T2 & r_outer,
                                const T3 & emiss_inner,
                                const T4 & emiss_outer,
                                const T5 & T_inner,
                                const T6 & T_outer,
                                const Real & sigma = HeatConduction::Constants::sigma)
{
  mooseAssert(r_outer > r_inner, "Outer radius must be larger than inner radius.");

  const auto rad_resistance =
      1.0 / emiss_inner + r_inner / r_outer * (1.0 - emiss_outer) / emiss_outer;
  return sigma * (std::pow(T_inner, 4) - std::pow(T_outer, 4)) / rad_resistance;
}

}
