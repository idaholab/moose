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

template <>
InputParameters
validParams<FunctionIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<FunctionName>("function", "The initial condition function.");
  return params;
}

FunctionIC::FunctionIC(const InputParameters & parameters)
  : InitialCondition(parameters), _func(getFunction("function"))
{
}

Real
FunctionIC::value(const Point & p)
{
  return _func.value(_t, p);
}

RealGradient
FunctionIC::gradient(const Point & p)
{
  return _func.gradient(_t, p);
}
