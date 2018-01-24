/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYCOREY_H
#define POROUSFLOWRELATIVEPERMEABILITYCOREY_H

#include "PorousFlowRelativePermeabilityBase.h"

class PorousFlowRelativePermeabilityCorey;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityCorey>();

/**
 * Material to calculate Corey-type relative permeability of an arbitrary phase
 * given the effective saturation and Corey exponent of that phase.
 *
 * From  Corey, A. T., The interrelation between gas and oil relative
 * permeabilities, Prod. Month. 38-41 (1954)
 */
class PorousFlowRelativePermeabilityCorey : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityCorey(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Corey exponent n for the specified phase
  const Real _n;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYCOREY_H
