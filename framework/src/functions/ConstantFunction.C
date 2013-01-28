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

#include "ConstantFunction.h"

template<> InputParameters validParams<ConstantFunction>()
{
   InputParameters params = validParams<Function>();
   params.addParam<Real>("value", 0.0, "The constant value");
   return params;
}

ConstantFunction::ConstantFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _value(getParam<Real>("value"))
{
}

Real
ConstantFunction::value(Real, const Point &)
{
  return _value;
}
