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

#ifndef CONSTANTFUNCTION_H
#define CONSTANTFUNCTION_H

#include "Function.h"

class ConstantFunction;

template<> InputParameters validParams<ConstantFunction>();

/**
 * Class that represents constant function
 */
class ConstantFunction : public Function
{
public:
  ConstantFunction(const std::string & name, InputParameters parameters);

  virtual Real value(Real t, const Point & p);

protected:
  Real _value;
};

#endif

