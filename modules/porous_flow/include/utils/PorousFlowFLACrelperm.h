//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
template <typename T>
T
relativePermeability(const T & seff, Real m)
{
  if (MetaPhysicL::raw_value(seff) <= 0.0)
    return 0.0;
  else if (MetaPhysicL::raw_value(seff) >= 1.0)
    return 1.0;

  using std::pow;
  return (1.0 + m) * pow(seff, m) - m * pow(seff, m + 1.0);
}

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
