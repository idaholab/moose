/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWHALFCUBICSINK_H
#define POROUSFLOWHALFCUBICSINK_H

#include "PorousFlowSinkPTDefiner.h"

// Forward Declarations
class PorousFlowHalfCubicSink;

template <>
InputParameters validParams<PorousFlowHalfCubicSink>();

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
  PorousFlowHalfCubicSink(const InputParameters & parameters);

protected:
  /// maximum of the cubic sink
  const Real _maximum;

  /// Denote x = porepressure - center.  Then Flux out = (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.  Flux out = max for x >= 0.  Flux out = 0 for x <= cutoff.
  Function & _cutoff;

  /// center of the cubic sink
  const Real _center;

  virtual Real multiplier() const override;

  virtual Real dmultiplier_dvar(unsigned int pvar) const override;
};

#endif // POROUSFLOWHALFCUBICSINK_H
