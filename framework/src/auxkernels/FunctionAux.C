//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionAux.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionAux);

InputParameters
FunctionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Auxiliary Kernel that creates and updates a field variable by "
                             "sampling a function through space and time.");
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  return params;
}

FunctionAux::FunctionAux(const InputParameters & parameters)
  : AuxKernel(parameters), _func(getFunction("function"))
{
}

Real
FunctionAux::computeValue()
{
  if (isNodal())
    return _func.value(_t, *_current_node);
  else
    return _func.value(_t, _q_point[_qp]);
}
