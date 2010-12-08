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

#include "ConstantAux.h"

template<>
InputParameters validParams<ConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("value", 0.0, "Some constant value that can be read from the input file");
  return params;
}

ConstantAux::ConstantAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _value(getParam<Real>("value"))
{}


Real
ConstantAux::computeValue()
{
  return _value;
}
