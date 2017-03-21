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

#include "ConstantScalarAux.h"

template <>
InputParameters
validParams<ConstantScalarAux>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addRequiredParam<Real>("value", "The value to be set to the scalar variable.");

  return params;
}

ConstantScalarAux::ConstantScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantScalarAux::computeValue()
{
  return _value;
}
