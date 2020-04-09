//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalVoronoi.h"
#include "MooseRandom.h"

// Forward Declarations

/**
 * PolycrystalHex creates a hexagonal polycrystal initial condition.
 * Only works for squared number of grains and with periodic BCs
 */
class PolycrystalHex : public PolycrystalVoronoi
{
public:
  static InputParameters validParams();

  PolycrystalHex(const InputParameters & parameters);

  virtual void precomputeGrainStructure();

private:
  const Real _x_offset;
  const Real _perturbation_percent;
  MooseRandom _random;
};
