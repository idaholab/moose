/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW2PHASEPS_VG_H
#define POROUSFLOW2PHASEPS_VG_H

#include "PorousFlow2PhasePS.h"

// Forward Declarations
class PorousFlow2PhasePS_VG;

template <>
InputParameters validParams<PorousFlow2PhasePS_VG>();

/**
 * Calculates porepressure and saturation at the nodes and qps using a van Genuchten
 * capillary pressure curve
 */
class PorousFlow2PhasePS_VG : public PorousFlow2PhasePS
{
public:
  PorousFlow2PhasePS_VG(const InputParameters & parameters);

protected:
  virtual Real capillaryPressure(Real seff) const override;

  virtual Real dCapillaryPressure_dS(Real seff) const override;

  virtual Real d2CapillaryPressure_dS2(Real seff) const override;

  /// van Genuchten exponent m
  const Real _m;
  /// Maximum capillary pressure (Pa). Note: must be <= 0
  const Real _pc_max;
  /// van Genuchten capillary pressure coefficient (inverse of alpha)
  const Real _p0;
};

#endif // POROUSFLOW2PHASEPS_VG_H
