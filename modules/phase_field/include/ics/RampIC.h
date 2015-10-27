/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RAMPIC_H
#define RAMPIC_H

#include "InitialCondition.h"
#include "InputParameters.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include <string>

//Forward Declarations
class RampIC;
class MooseMesh;

template<>
InputParameters validParams<RampIC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class RampIC : public InitialCondition
{
public:
  RampIC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  Real f();

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point &p);

  /**
   * The value of the gradient at a point.
   */
  virtual RealGradient gradient(const Point &p);

  Real _xlength;
  Real _xmin;
  Real _value_left;
  Real _value_right;
};

#endif //RAMPIC_H
