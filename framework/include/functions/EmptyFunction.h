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

#ifndef EMPTYFUNCTION_H
#define EMPTYFUNCTION_H

#include "Function.h"

class EmptyFunction;

template<>
InputParameters validParams<EmptyFunction>();

/**
 * Do nothing function
 */
class EmptyFunction : public Function
{
public:
  EmptyFunction(const std::string & name, InputParameters parameters);

  virtual ~EmptyFunction();

  virtual Real value(Real t, Real x, Real y = 0, Real z = 0);
};

#endif //EMPTYFUNCTION_H
