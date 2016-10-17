/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYVG_H
#define POROUSFLOWRELATIVEPERMEABILITYVG_H

#include "PorousFlowRelativePermeabilityBase.h"

class PorousFlowRelativePermeabilityVG;

template<>
InputParameters validParams<PorousFlowRelativePermeabilityVG>();

/**
 * Material to calculate van Genuchten-type relative permeability
 * of an arbitrary phase given the saturation and exponent of that phase.
 *
 * From van Genuchten, M. Th., A closed for equation for predicting the
 * hydraulic conductivity of unsaturated soils, Soil Sci. Soc., 44, 892-898 (1980)
 */
class PorousFlowRelativePermeabilityVG : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityVG(const InputParameters & parameters);

protected:
  virtual Real effectiveSaturation(Real saturation) const override;

  virtual Real relativePermeability(Real seff) const override;

  virtual Real dRelativePermeability_dS(Real seff) const override;

  /// van Genuchten exponent m for the specified phase
  const Real _m;
  /// Fully saturated phase saturation
  const Real _s_ls;
};

#endif //POROUSFLOWRELATIVEPERMEABILITYVG_H
