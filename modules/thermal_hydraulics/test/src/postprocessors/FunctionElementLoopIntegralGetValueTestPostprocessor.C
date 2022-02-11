//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionElementLoopIntegralGetValueTestPostprocessor.h"
#include "FunctionElementLoopIntegralUserObject.h"

registerMooseObject("ThermalHydraulicsTestApp", FunctionElementLoopIntegralGetValueTestPostprocessor);

InputParameters
FunctionElementLoopIntegralGetValueTestPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription("Gets the value from a FunctionElementLoopIntegralUserObject.");

  params.addRequiredParam<UserObjectName>("function_element_loop_integral_uo",
                                          "FunctionElementLoopIntegralUserObject name");

  return params;
}

FunctionElementLoopIntegralGetValueTestPostprocessor::
    FunctionElementLoopIntegralGetValueTestPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),

    _uo(getUserObject<FunctionElementLoopIntegralUserObject>("function_element_loop_integral_uo"))
{
}

void
FunctionElementLoopIntegralGetValueTestPostprocessor::initialize()
{
}

void
FunctionElementLoopIntegralGetValueTestPostprocessor::execute()
{
}

PostprocessorValue
FunctionElementLoopIntegralGetValueTestPostprocessor::getValue()
{
  return _uo.getValue();
}
