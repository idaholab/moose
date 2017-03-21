/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW1PHASEP_VG_H
#define POROUSFLOW1PHASEP_VG_H

#include "PorousFlow1PhaseP.h"
#include "PorousFlowVanGenuchten.h"

// Forward Declarations
class PorousFlow1PhaseP_VG;

template <>
InputParameters validParams<PorousFlow1PhaseP_VG>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, using a van-Genuchten capillary suction function
 */
class PorousFlow1PhaseP_VG : public PorousFlow1PhaseP
{
public:
  PorousFlow1PhaseP_VG(const InputParameters & parameters);

protected:
  Real effectiveSaturation(Real pressure) const override;

  Real dEffectiveSaturation_dP(Real pressure) const override;

  Real d2EffectiveSaturation_dP2(Real pressure) const override;

  /// van-Genuchten alpha parameter
  const Real _al;
  /// van-Genuchten m parameter
  const Real _m;
};

#endif // POROUSFLOW1PHASEP_VG_H
