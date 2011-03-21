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

#ifndef SPHEREFUNCTION_H
#define SPHEREFUNCTION_H

#include "Function.h"

class SphereFunction;

template<>
InputParameters validParams<SphereFunction>();

/**
 * This class will allow the user to provide values based on whether they lie inside or outside a spherical
 * surface in space.
 */
class SphereFunction : public Function
{
public:
  SphereFunction(const std::string & name, InputParameters parameters);

  virtual Real value(Real, const Point & p);

private:
  Point _center;
  Real _radius;
  Real _inside;
  Real _outside;
};

#endif //SPHEREFUNCTION_H
