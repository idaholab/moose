/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TRICRYSTALTRIPLEJUNCTIONIC_H
#define TRICRYSTALTRIPLEJUNCTIONIC_H

#include "InitialCondition.h"

// Forward Declarations
class TricrystalTripleJunctionIC;

template <>
InputParameters validParams<TricrystalTripleJunctionIC>();

/**
 * TricrystalTripleJunctionIC creates a 3-grain structure with a triple junction
 * centered at _junction as specified by the user.
 */
class TricrystalTripleJunctionIC : public InitialCondition
{
public:
  TricrystalTripleJunctionIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const MooseMesh & _mesh;

  /// Number of order parameters
  const unsigned int _op_num;

  // Order parameter index
  const unsigned int _op_index;

  /// Point where the triple junction occurs
  Point _junction;

  /// Angle of first grain at triple junction in radians
  Real _theta1;

  /// Angle of third grain at triple junction in radians
  Real _theta2;

  ///tangent of the first angle after a shift of pi/2
  Real _tan_theta1;

  ///tangent of the second angle after a shift of pi/2
  Real _tan_theta2;
};

#endif // TRICRYSTALTRIPLEJUNCTIONIC_H
