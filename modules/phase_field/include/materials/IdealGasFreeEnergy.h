/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
