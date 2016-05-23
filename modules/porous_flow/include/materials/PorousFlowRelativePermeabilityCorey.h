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

template<>
InputParameters validParams<PorousFlowRelativePermeabilityCorey>();

/**
 * Material designed to calculate Corey-type relative permeability
 * of an arbitrary phase given the saturation and Corey exponent of that phase
 */
class PorousFlowRelativePermeabilityCorey : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityCorey(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Corey exponent n for the specified phase
  const Real _n;
};

#endif //POROUSFLOWRELATIVEPERMEABILITYCOREY_H
