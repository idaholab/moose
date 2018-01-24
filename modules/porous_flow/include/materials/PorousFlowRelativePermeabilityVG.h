/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYVG_H
#define POROUSFLOWRELATIVEPERMEABILITYVG_H

#include "PorousFlowRelativePermeabilityBase.h"
#include "PorousFlowVanGenuchten.h"

class PorousFlowRelativePermeabilityVG;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityVG>();

/**
 * Material to calculate van Genuchten-type relative permeability
 * of an arbitrary phase given the saturation and exponent of that phase.
 *
 * From van Genuchten, M. Th., A closed for equation for predicting the
 * hydraulic conductivity of unsaturated soils, Soil Sci. Soc., 44, 892-898 (1980)
 *
 * Optionally this relative permeability may be smoothed with a cubic near seff=1
 * The relative permeability is a cubic for seff>_cut.  The cubic is chosen so
 * that the derivative is zero for seff=1, and that the derivative and value matches
 * the van Genuchten expression for seff=_cut.
 */
class PorousFlowRelativePermeabilityVG : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityVG(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// van Genuchten exponent m for the specified phase
  const Real _m;

  /// start of cubic smoothing
  const Real _cut;

  /// Parameter of the cubic
  const Real _cub0;
  /// Parameter of the cubic
  const Real _cub1;
  /// Parameter of the cubic
  const Real _cub2;
  /// Parameter of the cubic
  const Real _cub3;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYVG_H
