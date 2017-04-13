/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEXPOLYCRYSTALIC_H
#define HEXPOLYCRYSTALIC_H

#include "PolycrystalReducedIC.h"
#include "MooseRandom.h"

// Forward Declarations
class HexPolycrystalIC;

template <>
InputParameters validParams<HexPolycrystalIC>();

/**
 * HexPolycrystalIC creates a hexagonal polycrystal initial condition.
 * Only works for squared number of grains and with periodic BCs
 */
class HexPolycrystalIC : public PolycrystalReducedIC
{
public:
  HexPolycrystalIC(const InputParameters & parameters);

  virtual void initialSetup();

private:
  const Real _x_offset;
  const Real _perturbation_percent;
  MooseRandom _random;
};

#endif // HEXPOLYCRYSTALIC_H
