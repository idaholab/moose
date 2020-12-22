//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpecifiedSmoothSuperellipsoidIC.h"

/**
 * BimodalSuperellipsoidsIC takes a specified number of superellipsoids, each with given parameters
 * These are intended to be the larger particles. Then the IC creates a specified number
 * of particles at random locations. These are the smaller particles. As each random particle
 * is placed, it it checked to make sure it does not collide with previously placed particles
 * (either
 * large or small ones). Variables to describe the specified (larger) superellipsoids are inherited
 * from the parent class.
 */
class BimodalSuperellipsoidsIC : public SpecifiedSmoothSuperellipsoidIC
{
public:
  static InputParameters validParams();

  BimodalSuperellipsoidsIC(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidExponents();

  ///@{ Variables to describe the randomly placed (smaller) superellipsoids
  unsigned int _npart;
  Real _small_spac;
  Real _large_spac;
  Real _small_a;
  Real _small_b;
  Real _small_c;
  Real _small_n;
  const Real _size_variation;
  const MooseEnum _size_variation_type;
  ///@}

  const unsigned int _max_num_tries;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};
