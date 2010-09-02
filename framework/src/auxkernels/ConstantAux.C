/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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

ConstantAux::ConstantAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _value(getParam<Real>("value"))
{}


Real
ConstantAux::computeValue()
{
  return _value;
}
