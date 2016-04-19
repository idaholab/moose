/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BIMODALSUPERELLIPSOIDSIC_H
#define BIMODALSUPERELLIPSOIDSIC_H

#include "Kernel.h"
#include "SmoothSuperellipsoidBaseIC.h"

// System includes
#include <string>

// Forward Declarations
class BimodalSuperellipsoidsIC;

template<>
InputParameters validParams<BimodalSuperellipsoidsIC>();

/**
 * BimodalSuperellipsoidsIC takes a specified number of superellipsoids, each with given parameters
 * These are intended to be the larger particles. Then the IC creates a specified number
 * of particles at random locations. These are the smaller particles. As each random particle
 * is placed it it checked to make sure it does not collide with previously placed particles (either
 * large or small ones).
 **/
class BimodalSuperellipsoidsIC : public SmoothSuperellipsoidBaseIC
{
public:
  BimodalSuperellipsoidsIC(const InputParameters & parameters);

  virtual void initialSetup();
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidExponents();

protected:

  unsigned int _numbub;
  Real _bubspac;

  unsigned int _numtries;
  Real _radius;
  Real _radius_variation;
  MooseEnum _radius_variation_type;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};

#endif //BIMODALSUPERELLIPSOIDSIC_H
