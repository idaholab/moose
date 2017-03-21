/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW1PHASEP_BW_H
#define POROUSFLOW1PHASEP_BW_H

#include "PorousFlow1PhaseP.h"
#include "PorousFlowBroadbridgeWhite.h"

// Forward Declarations
class PorousFlow1PhaseP_BW;

template <>
InputParameters validParams<PorousFlow1PhaseP_BW>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, using a Broadbridge-White capillary suction function
 * P Broadbridge, I White ``Constant rate rainfall
 * infiltration: A versatile nonlinear model, 1 Analytical solution''.
 * Water Resources Research 24 (1988) 145--154.
 */
class PorousFlow1PhaseP_BW : public PorousFlow1PhaseP
{
public:
  PorousFlow1PhaseP_BW(const InputParameters & parameters);

protected:
  Real effectiveSaturation(Real pressure) const override;

  Real dEffectiveSaturation_dP(Real pressure) const override;

  Real d2EffectiveSaturation_dP2(Real pressure) const override;

  /// BW's Sn parameter (initial saturation)
  const Real _sn;

  /// BW's Ss parameter
  const Real _ss;

  /// BW's C parameter (>1)
  const Real _c;

  /// BWs lambda_s parameter multiplied yb fluiddensity * gravity (>0)
  const Real _las;
};

#endif // POROUSFLOW1PHASEP_BW_H
