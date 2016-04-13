/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PORFLOWMATERIALRELATIVEPERMEABILITYCOREY_H
#define PORFLOWMATERIALRELATIVEPERMEABILITYCOREY_H

#include "PorousFlowMaterialRelativePermeabilityBase.h"

class PorousFlowMaterialRelativePermeabilityCorey;

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityCorey>();

/**
 * Material designed to calculate Corey-type relative permeability
 * of an arbitrary phase given the saturation and
 * Corey exponent of that phase.
 */
class PorousFlowMaterialRelativePermeabilityCorey : public PorousFlowMaterialRelativePermeabilityBase
{
public:
  PorousFlowMaterialRelativePermeabilityCorey(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();

  /// Core exponent n for the phase
  const Real _n;
};

#endif //PORFLOWMATERIALRELATIVEPERMEABILITYCOREY_H
