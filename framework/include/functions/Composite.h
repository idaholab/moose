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

#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "Function.h"
#include "FunctionInterface.h"

class Composite;

template<>
InputParameters validParams<Composite>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class Composite : public Function, protected FunctionInterface
{
public:
  Composite(const std::string & name, InputParameters parameters);
  virtual ~Composite();

  virtual Real value(Real t, const Point & pt);

private:
  const Real _scale_factor;
  std::vector<Function *> _f;

};

#endif //COMPOSITE_H
