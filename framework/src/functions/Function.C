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

#include "Function.h"

template<>
InputParameters validParams<Function>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<std::string>("built_by_action", "add_function");
  return params;
}

Function::Function(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    TransientInterface(parameters, name, "functions"),
    PostprocessorInterface(parameters),
    UserObjectInterface(parameters),
    Restartable(name, parameters, "Functions")
{
}

Function::~Function()
{
}

Real
Function::value(Real /*t*/, const Point & /*p*/)
{
  return 0.0;
}

RealGradient
Function::gradient(Real /*t*/, const Point & /*p*/)
{
  return RealGradient(0, 0, 0);
}

RealVectorValue
Function::vectorValue(Real /*t*/, const Point & /*p*/)
{
  return RealVectorValue(0, 0, 0);
}

Real
Function::integral()
{
  mooseError("Integral method not defined for function " << _name);
  return 0;
}

Real
Function::average()
{
  mooseError("Average method not defined for function " << _name);
  return 0;
}
