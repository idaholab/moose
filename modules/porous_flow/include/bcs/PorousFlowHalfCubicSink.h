//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowSinkPTDefiner.h"

/**
 * Applies a flux sink to a boundary.  The base flux
 * defined by PorousFlowSink is multiplied by a cubic.
 * Denote x = porepressure - center, or in the case of
 * heat fluxes with no fluid, x = temperature - center.  Then
 * Then Flux out = (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.
 * Flux out = max for x >= 0.
 * Flux out = 0 for x <= cutoff.
 * This is typically used for modelling evapotranspiration
 * from the top of a groundwater model
 */
class PorousFlowHalfCubicSink : public PorousFlowSinkPTDefiner
{
public:
  static InputParameters validParams();

  PorousFlowHalfCubicSink(const InputParameters & parameters);

protected:
  /// Maximum of the cubic sink
  const Real _maximum;

  /// Denote x = porepressure - center.  Then Flux out = (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.  Flux out = max for x >= 0.  Flux out = 0 for x <= cutoff.
  const Function & _cutoff;

  /// Center of the cubic sink
  const Real _center;

  virtual Real multiplier() const override;

  virtual Real dmultiplier_dvar(unsigned int pvar) const override;
};
