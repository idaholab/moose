//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionPresetBC.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionPresetBC>()
{
  InputParameters params = validParams<PresetNodalBC>();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "The same as FunctionDirichletBC except the value is applied before the solve begins");
  return params;
}

FunctionPresetBC::FunctionPresetBC(const InputParameters & parameters)
  : PresetNodalBC(parameters), _func(getFunction("function"))
{
}

Real
FunctionPresetBC::computeQpValue()
{
  return _func.value(_t, *_current_node);
}
