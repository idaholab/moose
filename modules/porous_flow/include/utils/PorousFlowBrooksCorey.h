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

/**
 * Brooks-Corey effective saturation, capillary pressure and relative
 * permeability functions.
 *
 * Note: capillary pressure and relative permeability are functions of effective
 * saturation. The derivatives are therefore given wrt effective saturation. These
 * derivatives must be multiplied by the derivative of effective saturation wrt
 * the true saturation in objects using these relations.
 *
 * From Brooks, R. H. and A. T. Corey (1966), Properties of porous media affecting
 * fluid flow, J. Irrig. Drain. Div., 92, 61-88
 */

namespace PorousFlowBrooksCorey
{
/**
 * Effective saturation as a function of capillary pressure
 * Note: seff = 1 for p >= 0
 * @param pc capillary pressure
 * @param pe threshold entry pressure
 * @param lambda Brooks-Corey exponent
 * @return effective saturation
 */
Real effectiveSaturation(Real pc, Real pe, Real lambda);

/**
 * Derivative of effective saturation wrt porepressure
 * @param pc capillary pressure
 * @param pe threshold entry pressure
 * @param lambda Brooks-Corey exponent
 * @return derivative of effective saturation wrt porepressure
 */
Real dEffectiveSaturation(Real pc, Real pe, Real lambda);

/**
 * Second derivative of effective saturation wrt porepressure
 * @param pc capillary pressure
 * @param pe threshold entry pressure
 * @param lambda Brooks-Corey exponent
 * @return second derivative of effective saturation wrt porepressure
 */
Real d2EffectiveSaturation(Real pc, Real pe, Real lambda);

/**
 * Capillary pressure as a function of effective saturation
 *
 * @param seff effective saturation
 * @param pe threshold entry pressure
 * @param lambda Brooks-Corey exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return capillary pressure (Pa)
 */
Real capillaryPressure(Real seff, Real pe, Real lambda, Real pc_max);

/**
 * Derivative of capillary pressure wrt effective saturation
 *
 * @param seff effective saturation
 * @param pe threshold entry pressure
 * @param lambda Brooks-Corey exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return derivative of capillary pressure wrt effective saturation
 */
Real dCapillaryPressure(Real seff, Real pe, Real lambda, Real pc_max);

/**
 * Second derivative of capillary pressure wrt effective saturation
 *
 * @param seff effective saturation
 * @param pe threshold entry pressure
 * @param lambda Brooks-Corey exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return second derivative of capillary pressure wrt effective saturation
 */
Real d2CapillaryPressure(Real seff, Real pe, Real lambda, Real pc_max);

/**
 * Relative permeability of the wetting phase as a function of effective saturation
 * @param seff effective saturation
 * @param lambda Brooks-Corey exponent
 * @return relative permeability
 */
Real relativePermeabilityW(Real seff, Real lambda);

/**
 * Derivative of relative permeability of the wetting phase wrt to effective saturation
 * @param seff effective saturation
 * @param lambda Brooks-Corey exponent
 * @return derivative of relative permeability wrt effective saturation
 */
Real dRelativePermeabilityW(Real seff, Real lambda);

/**
 * Relative permeability of the non-wetting phase as a function of effective saturation
 * @param seff effective saturation
 * @param lambda Brooks-Corey exponent
 * @return relative permeability
 */
Real relativePermeabilityNW(Real seff, Real lambda);

/**
 * Derivative of relative permeability of the non-wetting phase wrt to effective saturation
 * @param seff effective saturation
 * @param lambda Brooks-Corey exponent
 * @return derivative of relative permeability wrt effective saturation
 */
Real dRelativePermeabilityNW(Real seff, Real lambda);
}
