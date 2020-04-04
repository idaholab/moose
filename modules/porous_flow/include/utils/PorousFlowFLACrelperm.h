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
