//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTION_H
#define FUNCTION_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "ScalarCoupleable.h"

// libMesh
#include "libmesh/vector_value.h"

// Forward declarations
class Function;

// libMesh forward declarations
namespace libMesh
{
class Point;
}

template <>
InputParameters validParams<Function>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class Function : public MooseObject,
                 public SetupInterface,
                 public TransientInterface,
                 public PostprocessorInterface,
                 public UserObjectInterface,
                 public Restartable,
                 public MeshChangedInterface,
                 public ScalarCoupleable
{
public:
  /**
   * Class constructor
   * \param parameters The input parameters for the function
   */
  Function(const InputParameters & parameters);

  /**
   * Function destructor
   */
  virtual ~Function();

  /**
   * Override this to evaluate the scalar function at point (t,x,y,z), by default
   * this returns zero, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A scalar of the function evaluated at the time and location
   */
  virtual Real value(Real t, const Point & p);

  /**
   * Override this to evaluate the vector function at a point (t,x,y,z), by default
   * this returns a zero vector, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A vector of the function evaluated at the time and location
   */
  virtual RealVectorValue vectorValue(Real t, const Point & p);

  /**
   * Function objects can optionally provide a gradient at a point. By default
   * this returns 0, you must override it.
   * \param t The time
   * \param p The Point in space (x,y,z)
   * \return A gradient of the function evaluated at the time and location
   */
  virtual RealGradient gradient(Real t, const Point & p);

  /**
   * Get the time derivative of the function
   * \param t The time
   * \param p The point in space (x,y,z)
   * \return The time derivative of the function at the specified time and location
   */
  virtual Real timeDerivative(Real t, const Point & p);

  // Not defined
  virtual Real integral();

  // Not defined
  virtual Real average();
};

#endif // FUNCTION_H
