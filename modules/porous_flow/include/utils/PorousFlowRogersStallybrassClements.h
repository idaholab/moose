/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWROGERSSTALLYBRASSCLEMENTS_H
#define POROUSFLOWROGERSSTALLYBRASSCLEMENTS_H

#include "MooseTypes.h"

/**
 * Rogers-Stallybrass-Clements version of effective saturation as a function of capillary pressure.
 * valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important
 * here!).
 * C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary
 * infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and
 * Applications 7 (1983) 785--799.
 */
namespace PorousFlowRogersStallybrassClements
{
/**
 * Effective saturation as a function of capillary pressure
 * @param pc capillary pressure
 * @param shift RSC's shift parameter
 * @param scale RSC's scale parameter
 * @return effective saturation
 */
Real effectiveSaturation(Real pc, Real shift, Real scale);

/**
 * Derivative of effective saturation wrt capillary pressure
 * @param pc capillary pressure
 * @param shift RSC's shift parameter
 * @param scale RSC's scale parameter
 * @return derivative of effective saturation wrt capillary pressure
 */
Real dEffectiveSaturation(Real pc, Real shift, Real scale);

/**
 * Second derivative of effective saturation wrt capillary pressure
 * @param pc capillary pressure
 * @param shift RSC's shift parameter
 * @param scale RSC's scale parameter
 * @return second derivative of effective saturation wrt capillary pressure
 */
Real d2EffectiveSaturation(Real pc, Real shift, Real scale);
}

#endif // POROUSFLOWROGERSSTALLYBRASSCLEMENTS_H
