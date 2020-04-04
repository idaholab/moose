//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseError.h"

/**
 * Broadbridge-White version of relative permeability,
 * and effective saturation as a function of capillary pressure.
 * Valid for Kn small.
 * P Broadbridge, I White ``Constant rate rainfall
 * infiltration: A versatile nonlinear model, 1 Analytical solution''.
 * Water Resources Research 24 (1988) 145--154.
 */
namespace PorousFlowBroadbridgeWhite
{
/**
 * Provides the Lambert W function, which satisfies
 * W(z) * exp(W(z)) = z
 * @param z z value, which must be positive
 * @return W
 */
Real LambertW(Real z);

/**
 * Effective saturation as a function of capillary pressure
 * If pc>=0 this will yield 1, otherwise it will yield <1
 * @param pc capillary pressure
 * @param c BW's C parameter
 * @param sn BW's S_n parameter
 * @param ss BW's S_s parameter
 * @param las BW's lambda_s parameter
 * @return effective saturation
 */
Real effectiveSaturation(Real pc, Real c, Real sn, Real ss, Real las);

/**
 * Derivative of effective saturation wrt capillary pressure
 * @param pc capillary pressure
 * @param c BW's C parameter
 * @param sn BW's S_n parameter
 * @param ss BW's S_s parameter
 * @param las BW's lambda_s parameter
 * @return derivative of effective saturation wrt capillary pressure
 */
Real dEffectiveSaturation(Real pc, Real c, Real sn, Real ss, Real las);

/**
 * Second derivative of effective saturation wrt capillary pressure
 * @param pc capillary pressure
 * @param c BW's C parameter
 * @param sn BW's S_n parameter
 * @param ss BW's S_s parameter
 * @param las BW's lambda_s parameter
 * @return second derivative of effective saturation wrt capillary pressure
 */
Real d2EffectiveSaturation(Real pc, Real c, Real sn, Real ss, Real las);

/**
 * Relative permeability as a function of saturation
 * @param s saturation
 * @param c BW's C parameter
 * @param sn BW's S_n parameter
 * @param ss BW's S_s parameter
 * @param kn BW's K_n parameter
 * @param ks BW's K_s parameter
 * @return relative permeability
 */
Real relativePermeability(Real s, Real c, Real sn, Real ss, Real kn, Real ks);

/**
 * Derivative of relative permeability with respect to saturation
 * @param s saturation
 * @param c BW's C parameter
 * @param sn BW's S_n parameter
 * @param ss BW's S_s parameter
 * @param kn BW's K_n parameter
 * @param ks BW's K_s parameter
 * @return derivative of relative permeability wrt saturation
 */
Real dRelativePermeability(Real s, Real c, Real sn, Real ss, Real kn, Real ks);

/**
 * Second derivative of relative permeability with respect to saturation
 * @param s saturation
 * @param c BW's C parameter
 * @param sn BW's S_n parameter
 * @param ss BW's S_s parameter
 * @param kn BW's K_n parameter
 * @param ks BW's K_s parameter
 * @return second derivative of relative permeability wrt saturation
 */
Real d2RelativePermeability(Real s, Real c, Real sn, Real ss, Real kn, Real ks);
}
