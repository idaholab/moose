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

template <>
InputParameters
validParams<Function>()
{
  InputParameters params = validParams<MooseObject>();

  params.registerBase("Function");

  return params;
}

Function::Function(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    UserObjectInterface(this),
    Restartable(parameters, "Functions"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this)
{
}

Function::~Function() {}

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

Real
Function::timeDerivative(Real /*t*/, const Point & /*p*/)
{
  mooseError("timeDerivative method not defined for function ", name());
  return 0;
}

RealVectorValue
Function::vectorValue(Real /*t*/, const Point & /*p*/)
{
  return RealVectorValue(0, 0, 0);
}

Real
Function::integral()
{
  mooseError("Integral method not defined for function ", name());
  return 0;
}

Real
Function::average()
{
  mooseError("Average method not defined for function ", name());
  return 0;
}
