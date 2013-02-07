/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef FUNCTION_H
#define FUNCTION_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "UserObjectInterface.h"

// libMesh
#include "libmesh/vector_value.h"
#include "libmesh/point.h"

class Function;

template<>
InputParameters validParams<Function>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class Function :
  public MooseObject,
  public SetupInterface,
  public TransientInterface,
  public PostprocessorInterface,
  public UserObjectInterface
{
public:
  Function(const std::string & name, InputParameters parameters);
  virtual ~Function();

  /**
   * Override this to evaluate the function at point (t,x,y,z)
   */
  virtual Real value(Real t, const Point & p) = 0;

  /**
   * Function objects can optionally provide a gradient at a point. By default
   * this returns 0, you must override it.
   */
  virtual RealGradient gradient(Real t, const Point & p);

  virtual Real integral();

  virtual Real average();
};

#endif //FUNCTION_H
