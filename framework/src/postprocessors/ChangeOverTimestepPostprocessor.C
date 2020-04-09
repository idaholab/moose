//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChangeOverTimestepPostprocessor.h"

registerMooseObject("MooseApp", ChangeOverTimestepPostprocessor);

InputParameters
ChangeOverTimestepPostprocessor::validParams()
{
  InputParameters params = ChangeOverTimePostprocessor::validParams();
  return params;
}

ChangeOverTimestepPostprocessor::ChangeOverTimestepPostprocessor(const InputParameters & parameters)
  : ChangeOverTimePostprocessor(parameters)
{
  mooseDeprecated(
      "'ChangeOverTimestepPostprocessor' has been renamed to 'ChangeOverTimePostprocessor'");
}
