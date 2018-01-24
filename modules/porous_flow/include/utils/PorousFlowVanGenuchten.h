//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWVANGENUCHTEN_H
#define POROUSFLOWVANGENUCHTEN_H

#include "MooseTypes.h"

/**
 * van Genuchten effective saturation, capillary pressure and relative
 * permeability functions.
 * Note: effective saturation is provided as a function of porepressure, not
 * capillary pressure.
 * Note: capillary pressure and relative permeability are functions of effective
 * saturation. The derivatives are therefore given wrt effective saturation. These
 * derivatives must be multiplied by the derivative of effective saturation wrt
 * the true saturation in objects using these relations.
 *
 * Based on van Genuchten, M. Th., A closed for equation for
 * predicting the hydraulic conductivity of unsaturated soils,
 * Soil Sci. Soc., 44, 892-898 (1980).
 */

namespace PorousFlowVanGenuchten
{
/**
 * Effective saturation as a function of porepressure.
 * Note: seff = 1 for p >= 0
 * @param p porepressure
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @return effective saturation
 */
Real effectiveSaturation(Real p, Real alpha, Real m);

/**
 * Derivative of effective saturation wrt porepressure
 * @param p porepressure
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @return derivative of effective saturation wrt porepressure
 */
Real dEffectiveSaturation(Real p, Real alpha, Real m);

/**
 * Second derivative of effective saturation wrt porepressure
 * @param p porepressure
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @return second derivative of effective saturation wrt porepressure
 */
Real d2EffectiveSaturation(Real p, Real alpha, Real m);

/**
 * Capillary pressure as a function of effective saturation
 *
 * @param seff effective saturation
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return capillary pressure (Pa)
 */
Real capillaryPressure(Real seff, Real alpha, Real m, Real pc_max);

/**
 * Derivative of capillary pressure wrt effective saturation
 *
 * @param seff effective saturation
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return derivative of capillary pressure wrt effective saturation
 */
Real dCapillaryPressure(Real seff, Real alpha, Real m, Real pc_max);

/**
 * Second derivative of capillary pressure wrt effective saturation
 *
 * @param seff effective saturation
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return second derivative of capillary pressure wrt effective saturation
 */
Real d2CapillaryPressure(Real seff, Real alpha, Real m, Real pc_max);

/**
 * Relative permeability as a function of effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return relative permeability
 */
Real relativePermeability(Real seff, Real m);

/**
 * Derivative of relative permeability with respect to effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return derivative of relative permeability wrt effective saturation
 */
Real dRelativePermeability(Real seff, Real m);

/**
 * Second derivative of relative permeability with respect to effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return second derivative of relative permeability wrt effective saturation
 */
Real d2RelativePermeability(Real seff, Real m);
}

#endif // POROUSFLOWVANGENUCHTEN_H
