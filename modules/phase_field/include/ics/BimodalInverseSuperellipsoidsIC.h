/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BIMODALINVERSESUPERELLIPSOIDSIC_H
#define BIMODALINVERSESUPERELLIPSOIDSIC_H

#include "BimodalSuperellipsoidsIC.h"

// Forward Declarations
class BimodalInverseSuperellipsoidsIC;

template <>
InputParameters validParams<BimodalInverseSuperellipsoidsIC>();

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
  BimodalInverseSuperellipsoidsIC(const InputParameters & parameters);

  /// Have to do things slightly different from SmoothSuperellipsoidBaseIC because of the inverse structure
  virtual Real value(const Point & p);

  virtual void initialSetup();
  virtual void computeSuperellipsoidCenters();
};

#endif // BIMODALINVERSESUPERELLIPSOIDSIC_H
