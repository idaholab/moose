/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYCOREY_H
#define POROUSFLOWRELATIVEPERMEABILITYCOREY_H

#include "PorousFlowRelativePermeabilityUnity.h"

class PorousFlowRelativePermeabilityCorey;

template<>
InputParameters validParams<PorousFlowRelativePermeabilityCorey>();

/**
 * Material designed to calculate Corey-type relative permeability
 * of an arbitrary phase given the saturation and
 * Corey exponent of that phase.
 */
class PorousFlowRelativePermeabilityCorey : public PorousFlowRelativePermeabilityUnity
{
public:
  PorousFlowRelativePermeabilityCorey(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Core exponent n for the phase
  const Real _n;
};

#endif //POROUSFLOWRELATIVEPERMEABILITYCOREY_H
