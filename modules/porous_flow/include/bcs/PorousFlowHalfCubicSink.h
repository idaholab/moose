/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWHALFCUBICSINK_H
#define POROUSFLOWHALFCUBICSINK_H

#include "PorousFlowSink.h"

// Forward Declarations
class PorousFlowHalfCubicSink;

template<>
InputParameters validParams<PorousFlowHalfCubicSink>();

/**
 * Applies a flux sink to a boundary.  The base flux
 * defined by PorousFlowSink is multiplied by a cubic.
 * Denote x = porepressure - centre.  Then
 * Then Flux out = (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.
 * Flux out = max for x >= 0.
 * Flux out = 0 for x <= cutoff.
 * This is typically used for modelling evapotranspiration
 * from the top of a groundwater model
 */
class PorousFlowHalfCubicSink : public PorousFlowSink
{
public:

  PorousFlowHalfCubicSink(const InputParameters & parameters);

protected:
  /// maximum of the cubic sink
  const Real _maximum;

  /// Denote x = porepressure - centre.  Then Flux out = (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.  Flux out = max for x >= 0.  Flux out = 0 for x <= cutoff.
  Function & _cutoff;

  /// centre of the cubic sink
  const Real _centre;

  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real> > & _pp;

  /// d(Nodal pore pressure in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dpp_dvar;

  virtual Real multiplier();

  virtual Real dmultiplier_dvar(unsigned int pvar);
};

#endif //POROUSFLOWHALFCUBICSINK_H
