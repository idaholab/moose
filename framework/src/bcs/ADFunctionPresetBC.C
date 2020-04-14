//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctionPresetBC.h"
#include "Function.h"

registerMooseObjectDeprecated("MooseApp", ADFunctionPresetBC, "06/30/2020 24:00");

InputParameters
ADFunctionPresetBC::validParams()
{
  InputParameters params = ADDirichletBCBase::validParams();

  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "The same as ADFunctionDirichletBC except the value is applied before the solve begins");

  // Utilize the new ADDirichletBCBase with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

ADFunctionPresetBC::ADFunctionPresetBC(const InputParameters & parameters)
  : ADDirichletBCBase(parameters), _func(getFunction("function"))
{
  mooseDeprecated("Use ADFunctionDirichletBC with preset = true instead of ADFunctionPresetBC");
}

ADReal
ADFunctionPresetBC::computeQpValue()
{
  return _func.value(_t, *_current_node);
}
