//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RandomICBase.h"

/**
 * PFCFreezingIC creates an initial density for a PFC model that has one area of a set
 * crystal structure (initialized using sinusoids) and all the rest with a random structure.
 * The random values will fall between 0 and 1.
 * \todo For the FCC this returns 0. This cannot be right, yet it satisfies the (probably bogus)
 * test.
 */
class PFCFreezingIC : public RandomICBase
{
public:
  static InputParameters validParams();

  PFCFreezingIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

private:
  Real _x1;
  Real _y1;
  Real _z1;

  Real _x2;
  Real _y2;
  Real _z2;

  Real _lc;
  MooseEnum _crystal_structure;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  Real _min, _max, _val_range;
  Real _inside, _outside;

  unsigned int _icdim;
};
