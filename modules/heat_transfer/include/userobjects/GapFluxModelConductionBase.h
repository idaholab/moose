//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelBase.h"

/**
 * Gap flux model for varying gap conductance. Defines conduction physics using the \p
 * computeConductionFlux method
 */
class GapFluxModelConductionBase : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  GapFluxModelConductionBase(const InputParameters & parameters);

protected:
  /**
   * computes the conduction flux based on the input secondary and primary temperatures as well a
   * gap conductivity multiplier that multiplies the constant \p _gap_conductivity
   */
  ADReal computeConductionFlux(const ADReal & secondary_T,
                               const ADReal & primary_T,
                               const ADReal & conductivity_multiplier) const;

  ADReal gapAttenuation() const;

  /// Gap conductivity constant
  const Real _gap_conductivity;

  const Real _min_gap;

  const unsigned int _min_gap_order;
};
