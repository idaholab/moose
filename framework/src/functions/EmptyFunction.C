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

#include "Moose.h" //for mooseError
#include "EmptyFunction.h"

template<>
InputParameters validParams<EmptyFunction>()
{
  InputParameters params = validParams<Function>();
  return params;
}

EmptyFunction::EmptyFunction(const std::string & name, InputParameters parameters):
  Function(name, parameters)
{
}

EmptyFunction::~EmptyFunction()
{
}

Real
EmptyFunction::value(Real /*t*/, Real /*x*/, Real /*y*/, Real /*z*/)
{
  return 0.0;
}
