/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW2PHASEPP_RSC_H
#define POROUSFLOW2PHASEPP_RSC_H

#include "PorousFlow2PhasePP.h"
#include "PorousFlowRogersStallybrassClements.h"

// Forward Declarations
class PorousFlow2PhasePP_RSC;

template <>
InputParameters validParams<PorousFlow2PhasePP_RSC>();

/**
 * Material designed to calculate 2-phase porepressures and saturations at nodes and quadpoints
 * assuming the independent variables are the 2 porepressure, and
 * using the Rogers-Stallybrass-Clements capillary curve
 */
class PorousFlow2PhasePP_RSC : public PorousFlow2PhasePP
{
public:
  PorousFlow2PhasePP_RSC(const InputParameters & parameters);

protected:
  Real effectiveSaturation(Real pressure) const override;

  Real dEffectiveSaturation_dP(Real pressure) const override;

  Real d2EffectiveSaturation_dP2(Real pressure) const override;

  /// oil viscosity (which must be twice the water viscocity in this formulation)
  const Real _oil_viscosity;

  /// scale ratio: porosity/permeability/beta^2, where beta is chosen by the user
  const Real _scale_ratio;

  /// shift.  seff_water = 1/Sqrt(1 + Exp((Pc - shift)/scale)), where scale = 0.25 * scale_ratio * oil_viscosity
  const Real _shift;

  /// scale = 0.25 * scale_ratio * oil_viscosity
  const Real _scale;
};

#endif // POROUSFLOW2PHASEPP_RSC_H
