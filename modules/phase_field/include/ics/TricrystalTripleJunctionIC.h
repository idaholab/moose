/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TRICRYSTALTRIPLEJUNCTIONIC_H
#define TRICRYSTALTRIPLEJUNCTIONIC_H

// MOOSE Includes
#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class TricrystalTripleJunctionIC;

template<>
InputParameters validParams<TricrystalTripleJunctionIC>();

/**
 * TricrystalTripleJunctionIC creates a 3-grain structure with a triple junction in the center
 */
class TricrystalTripleJunctionIC : public InitialCondition
{
public:
  TricrystalTripleJunctionIC(const InputParameters & parameters);
  virtual Real value(const Point & p);

protected:
  MooseMesh & _mesh;
  /// A reference to the nonlinear system
  NonlinearSystem & _nl;

  unsigned int _op_num;
  unsigned int _op_index;

  Point _bottom_left;
  Point _top_right;
  Point _range;
  Real _theta1; // Angle of first grain at triple junction in radians
  Real _theta2; // Angle of third grain at triple junction in radians
  Real _tan_theta1; //tangent of the first angle after a shift of pi/2
  Real _tan_theta2; //tangent of the second angle after a shift of pi/2
};

#endif //TRICRYSTALTRIPLEJUNCTIONIC_H
