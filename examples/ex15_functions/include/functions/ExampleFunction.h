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

#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "Function.h"

class ExampleFunction;

template<>
InputParameters validParams<ExampleFunction>();

class ExampleFunction : public Function
{
public:
  ExampleFunction(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual Real value(Real t, Real x, Real y = 0, Real z = 0);

protected:
  Real _alpha;
};

#endif //EXAMPLEFUNCTION_H
