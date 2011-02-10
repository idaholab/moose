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

#include "FunctionIC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<std::string>("function", "The initial condition function.");
  return params;
}

FunctionIC::FunctionIC(const std::string & name,
                       InputParameters parameters)
  :InitialCondition(name, parameters),
  _func(getFunction("function"))
{}

Real
FunctionIC::value(const Point & p)
{
  return _func.value(0, p);
}
