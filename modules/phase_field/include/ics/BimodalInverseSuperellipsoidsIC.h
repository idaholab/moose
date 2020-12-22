//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BimodalSuperellipsoidsIC.h"

/**
 * BimodalInverseSuperellipsoidsIC takes a specified number of superellipsoids, each with given
 *parameters
 * These are intended to be the larger particles. Then the IC creates a specified number
 * of particles at random locations. These are the smaller particles. Unlike in the parent class,
 *the smaller
 * particles are embedded inside the larger particles, which is why this IC is referred to as
 *Inverse.
 **/
class BimodalInverseSuperellipsoidsIC : public BimodalSuperellipsoidsIC
{
public:
  static InputParameters validParams();

  BimodalInverseSuperellipsoidsIC(const InputParameters & parameters);

  /// Have to do things slightly different from SmoothSuperellipsoidBaseIC because of the inverse structure
  virtual Real value(const Point & p);

  virtual void initialSetup();
  virtual void computeSuperellipsoidCenters();
};
