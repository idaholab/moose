/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBEVOLUTION_H
#define GBEVOLUTION_H

#include "GBEvolutionBase.h"

// Forward Declarations
class GBEvolution;

template <>
InputParameters validParams<GBEvolution>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class GBEvolution : public GBEvolutionBase
{
public:
  GBEvolution(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  Real _GBEnergy;
};

#endif // GBEVOLUTION_H
