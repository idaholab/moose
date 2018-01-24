//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IDEALGASFREEENERGY_H
#define IDEALGASFREEENERGY_H

#include "GasFreeEnergyBase.h"

// Forward Declarations
class IdealGasFreeEnergy;

template <>
InputParameters validParams<IdealGasFreeEnergy>();

/**
 * Material class that provides the free energy of an ideal gas with the expression builder
 * and uses automatic differentiation to get the derivatives.
 */
class IdealGasFreeEnergy : public GasFreeEnergyBase
{
public:
  IdealGasFreeEnergy(const InputParameters & parameters);
};

#endif // IDEALGASFREEENERGY_H
