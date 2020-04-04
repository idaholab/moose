//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFunctionAux.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorFunctionAux);

InputParameters
VectorFunctionAux::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription(
      "Auxiliary Kernel that creates and updates a vector field variable by "
      "sampling a Function object, via the vectorValue method, through space and time.");
  params.addRequiredParam<FunctionName>("function", "The function to use as the value.");
  return params;
}

VectorFunctionAux::VectorFunctionAux(const InputParameters & parameters)
  : VectorAuxKernel(parameters), _function(getFunction("function"))
{
}

RealVectorValue
VectorFunctionAux::computeValue()
{
  if (isNodal())
    return _function.vectorValue(_t, *_current_node);
  else
    return _function.vectorValue(_t, _q_point[_qp]);
}
