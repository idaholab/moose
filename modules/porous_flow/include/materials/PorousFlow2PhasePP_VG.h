/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW2PHASEPP_VG_H
#define POROUSFLOW2PHASEPP_VG_H

#include "PorousFlow2PhasePP.h"
#include "PorousFlowVanGenuchten.h"

// Forward Declarations
class PorousFlow2PhasePP_VG;

template <>
InputParameters validParams<PorousFlow2PhasePP_VG>();

/**
 * Material designed to calculate 2-phase porepressures and saturations at nodes and quadpoints
 * assuming the independent variables are the 2 porepressure, and
 * using a van-Genuchten expression
 */
class PorousFlow2PhasePP_VG : public PorousFlow2PhasePP
{
public:
  PorousFlow2PhasePP_VG(const InputParameters & parameters);

protected:
  Real effectiveSaturation(Real pressure) const override;

  Real dEffectiveSaturation_dP(Real pressure) const override;

  Real d2EffectiveSaturation_dP2(Real pressure) const override;

  /// van Genuchten parameter alpha
  const Real _al;

  /// van Genuchten exponent m
  const Real _m;
};

#endif // POROUSFLOW2PHASEPP_VG_H
