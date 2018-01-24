/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLACRELPERM_H
#define POROUSFLOWFLACRELPERM_H

#include "MooseTypes.h"

/**
 * FLAC inspired relative permeability relationship
 */

namespace PorousFlowFLACrelperm
{
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

#endif // POROUSFLOWFLACRELPERM_H
