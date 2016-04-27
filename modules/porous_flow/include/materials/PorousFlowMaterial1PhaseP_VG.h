/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIAL1PHASEP_VG_H
#define POROUSFLOWMATERIAL1PHASEP_VG_H

#include "PorousFlowMaterial1PhaseP.h"
#include "PorousFlowEffectiveSaturationVG.h"

//Forward Declarations
class PorousFlowMaterial1PhaseP_VG;

template<>
InputParameters validParams<PorousFlowMaterial1PhaseP_VG>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, using a van-Genuchten capillary suction function
 */
class PorousFlowMaterial1PhaseP_VG : public PorousFlowMaterial1PhaseP
{
public:
  PorousFlowMaterial1PhaseP_VG(const InputParameters & parameters);

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

  /// van-Genuchten alpha parameter
  const Real _al;
  /// van-Genuchten m parameter
  const Real _m;
};

#endif //POROUSFLOWMATERIAL1PHASEP_VG_H
