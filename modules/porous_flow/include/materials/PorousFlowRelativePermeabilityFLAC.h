/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYFLAC_H
#define POROUSFLOWRELATIVEPERMEABILITYFLAC_H

#include "PorousFlowRelativePermeabilityBase.h"
#include "PorousFlowFLACrelperm.h"

class PorousFlowRelativePermeabilityFLAC;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityFLAC>();

/**
 * Material to calculate relative permeability inspired by the
 * formula used in FLAC:
 * relperm = (1 + m) seff^m - m seff^(m + 1)
 */
class PorousFlowRelativePermeabilityFLAC : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityFLAC(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// exponent m for the specified phase
  const Real _m;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYFLAC_H
