/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BIMODALSUPERELLIPSOIDSIC_H
#define BIMODALSUPERELLIPSOIDSIC_H

#include "SpecifiedSmoothSuperellipsoidIC.h"

// Forward Declarations
class BimodalSuperellipsoidsIC;

template <>
InputParameters validParams<BimodalSuperellipsoidsIC>();

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

#endif // BIMODALSUPERELLIPSOIDSIC_H
