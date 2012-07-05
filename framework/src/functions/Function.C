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
#include "Moose.h"

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
    TransientInterface(parameters),
    PostprocessorInterface(parameters),
    UserObjectInterface(parameters)
{
}

Function::~Function()
{
}

RealGradient
Function::gradient(Real /*t*/, const Point & /*p*/)
{
  return RealGradient(0, 0, 0);
}

Real
Function::integral()
{
  std::string errorMsg("Integral method not defined for function ");
  errorMsg += _name;
  mooseError( errorMsg );
  return 0;
}

Real
Function::average()
{
  std::string errorMsg("Average method not defined for function ");
  errorMsg += _name;
  mooseError( errorMsg );
  return 0;
}
