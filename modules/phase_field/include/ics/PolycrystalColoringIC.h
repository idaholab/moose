/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALCOLORINGIC_H
#define POLYCRYSTALCOLORINGIC_H

#include "InitialCondition.h"
#include "PolycrystalICTools.h"

// Forward Declarations
class PolycrystalColoringIC;
class GrainTrackerInterface;
class PolycrystalUserObjectBase;

template <>
InputParameters validParams<PolycrystalColoringIC>();

/**
 * PolycrystalColoringIC creates a polycrystal initial condition.
 * With 2 Grains, _typ = 0 results in a circular inclusion grain and _type = 1 gives a bicrystal.
 * With more than 2 grains, _typ = 0 gives set positions for 6 grains, _type = 1 gives hexagonal
 * grains for 4 grains.
 *                          _typ = 2 Gives a random voronoi structure
 */
class PolycrystalColoringIC : public InitialCondition
{
public:
  PolycrystalColoringIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  unsigned int _op_index;
  unsigned int _phase;
  const PolycrystalUserObjectBase & _poly_ic_uo;
};

#endif // POLYCRYSTALCOLORINGIC_H
