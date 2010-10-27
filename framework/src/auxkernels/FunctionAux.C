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

#include "FunctionAux.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("function", "The function to use as the value");
  return params;
}

FunctionAux::FunctionAux(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _func(getFunction("function"))
{}

Real
FunctionAux::computeValue()
{
  return _func.value(_t, (*_current_node)(0), (*_current_node)(1), (*_current_node)(2));
}
