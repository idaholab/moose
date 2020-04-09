//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "PolycrystalICTools.h"

// Forward Declarations
class GrainTrackerInterface;
class PolycrystalUserObjectBase;

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
  static InputParameters validParams();

  PolycrystalColoringIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  unsigned int _op_index;
  unsigned int _phase;
  const PolycrystalUserObjectBase & _poly_ic_uo;
};
