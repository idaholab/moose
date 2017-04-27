/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALHEX_H
#define POLYCRYSTALHEX_H

#include "PolycrystalVoronoi.h"
#include "MooseRandom.h"

// Forward Declarations
class PolycrystalHex;

template <>
InputParameters validParams<PolycrystalHex>();

/**
 * PolycrystalHex creates a hexagonal polycrystal initial condition.
 * Only works for squared number of grains and with periodic BCs
 */
class PolycrystalHex : public PolycrystalVoronoi
{
public:
  PolycrystalHex(const InputParameters & parameters);

  virtual void precomputeGrainStructure();

private:
  const Real _x_offset;
  const Real _perturbation_percent;
  MooseRandom _random;
};

#endif // POLYCRYSTALHEX_H
