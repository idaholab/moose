/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSUREVG_H
#define POROUSFLOWCAPILLARYPRESSUREVG_H

#include "MooseTypes.h"

/**
 * van Genuchten capillary pressure for multiphase flow in porous media.
 *
 * Based on van Genuchten, M. Th., A closed for equation for
 * predicting the hydraulic conductivity of unsaturated soils,
 * Soil Sci. Soc., 44, 892-898 (1980).
 */
namespace PorousFlowCapillaryPressureVG
{
/**
 * Effective saturation used in calculations
 *
 * @param s saturation
 * @param sat_r residual saturation
 * @param sat_s fully saturated saturation
 * @return effective saturation
 */
Real effectiveSaturation(Real s, Real sat_r, Real sat_l);

/**
 * Capillary pressure as a function of saturation
 *
 * @param s saturation
 * @param m van Genuchten exponent m
 * @param sat_r residual saturation
 * @param sat_s fully saturated saturation
 * @param p0 capillary pressure strength factor (Pa)
 * @param pc_max maximum capillary pressure (Pa)
 * @return capillary pressure (Pa)
 */
Real capillaryPressure(Real s, Real m, Real sat_r, Real sat_s, Real p0, Real pc_max);

/**
 * Derivative of capillary pressure wrt saturation
 *
 * @param s saturation
 * @param m van Genuchten exponent m
 * @param sat_r residual saturation
 * @param sat_s fully saturated saturation
 * @param p0 capillary pressure strength factor (Pa)
 * @param pc_max maximum capillary pressure (Pa)
 * @return d(capillary pressure)/ds (Pa)
 */
Real dCapillaryPressure(Real s, Real m, Real sat_r, Real sat_s, Real p0, Real pc_max);

/**
 * Second derivative of capillary pressure wrt saturation
 *
 * @param s saturation
 * @param m van Genuchten exponent m
 * @param sat_r residual saturation
 * @param sat_s fully saturated saturation
 * @param p0 capillary pressure strength factor (Pa)
 * @param pc_max maximum capillary pressure (Pa)
 * @return d^2(capillary pressure)/ds^2 (Pa)
 */
Real d2CapillaryPressure(Real s, Real m, Real sat_r, Real sat_s, Real p0, Real pc_max);
}

#endif // POROUSFLOWCAPILLARYPRESSUREVG_H
