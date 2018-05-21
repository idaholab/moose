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

registerMooseObject("MooseApp", FunctionPresetBC);

template <>
InputParameters
validParams<FunctionPresetBC>()
{
  InputParameters params = validParams<PresetNodalBC>();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addParam<bool>("incremental", 
                        false, 
                        "if true, the BCs is computed incrementally"
                        "from the result of the previous time step");
  params.addClassDescription(
      "The same as FunctionDirichletBC except the value is applied before the solve begins");
  return params;
}

FunctionPresetBC::FunctionPresetBC(const InputParameters & parameters)
  : PresetNodalBC(parameters), 
    _func(getFunction("function")),
    _incremental(getParam<bool>("incremental")),
    _u_old(_incremental ? &_var.dofValuesOld() : nullptr)
{
}

Real
FunctionPresetBC::computeQpValue()
{
  if (_incremental)
    return (*_u_old)[_qp] + (_func.value(_t, *_current_node) - _func.value(_t - _dt, *_current_node));

  return _func.value(_t, *_current_node);
}
