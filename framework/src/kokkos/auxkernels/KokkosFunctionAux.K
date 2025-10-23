//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionAux.h"

registerKokkosAuxKernel("MooseApp", KokkosFunctionAux);

InputParameters
KokkosFunctionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Auxiliary Kernel that creates and updates a field variable by "
                             "sampling a function through space and time.");
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  return params;
}

KokkosFunctionAux::KokkosFunctionAux(const InputParameters & parameters)
  : AuxKernel(parameters), _func(getKokkosFunction("function"))
{
}
