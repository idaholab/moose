//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
