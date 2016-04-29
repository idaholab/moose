/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PORFLOWMATERIAL2PHASEPP_VG_H
#define PORFLOWMATERIAL2PHASEPP_VG_H

#include "PorousFlowMaterial2PhasePP.h"
#include "PorousFlowEffectiveSaturationVG.h"

//Forward Declarations
class PorousFlowMaterial2PhasePP_VG;

template<>
InputParameters validParams<PorousFlowMaterial2PhasePP_VG>();

/**
 * Material designed to calculate fluid-phase porepressures and saturations at nodes and quadpoints
 */
class PorousFlowMaterial2PhasePP_VG : public PorousFlowMaterial2PhasePP
{
public:
  PorousFlowMaterial2PhasePP_VG(const InputParameters & parameters);

protected:
  /**
   * Effective saturation as a function of porepressure using van Genuchten
   * formulation.
   *
   * @param pressure porepressure (Pa)
   * @return effective saturation
   */
  Real effectiveSaturation(Real pressure) const;

  /**
   * Derivative of effective saturation wrt to porepressure.
   *
   * @param pressure porepressure (Pa)
   * @return derivative of effective saturation wrt porepressure
   */
  Real dEffectiveSaturation_dP(Real pressure) const;

  /**
   * Second derivative of effective saturation wrt to porepressure.
   *
   * @param pressure porepressure (Pa)
   * @return second derivative of effective saturation wrt porepressure
   */
  Real d2EffectiveSaturation_dP2(Real pressure) const;

  /// vanGenuchten alpha
  const Real _al;
  /// vanGenuchten m
  const Real _m;
};

#endif //PORFLOWMATERIAL2PHASEPP_VG_H
