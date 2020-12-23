//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothCircleBaseIC.h"

/**
 * LatticeSmoothcircleIC creates a lattice of smoothcircles as an initial condition.
 * They are either directly on the lattice or randomly perturbed from the lattice.
 */
class LatticeSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  static InputParameters validParams();

  LatticeSmoothCircleIC(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

  Real _lattice_variation;
  std::vector<unsigned int> _circles_per_side;
  unsigned int _numbub;

  Real _radius;
  Real _radius_variation;
  MooseEnum _radius_variation_type;

  bool _avoid_bounds;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};
