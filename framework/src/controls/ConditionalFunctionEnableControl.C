//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConditionalFunctionEnableControl.h"
#include "Function.h"

registerMooseObject("MooseApp", ConditionalFunctionEnableControl);

InputParameters
ConditionalFunctionEnableControl::validParams()
{
  InputParameters params = ConditionalEnableControl::validParams();

  params.addRequiredParam<FunctionName>("conditional_function",
                                        "The function to give a true or false value");

  params.addClassDescription(
      "Control for enabling/disabling objects when a function value is true");

  return params;
}

ConditionalFunctionEnableControl::ConditionalFunctionEnableControl(
    const InputParameters & parameters)
  : ConditionalEnableControl(parameters), _function(getFunction("conditional_function"))
{
}

bool
ConditionalFunctionEnableControl::conditionMet(const unsigned int & /*i*/)
{
  return _function.value(_t);
}
