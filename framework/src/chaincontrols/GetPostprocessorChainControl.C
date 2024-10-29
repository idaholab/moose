//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetPostprocessorChainControl.h"

registerMooseObject("MooseApp", GetPostprocessorChainControl);

InputParameters
GetPostprocessorChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();
  params.addRequiredParam<PostprocessorName>("postprocessor", "Post-processor name");
  params.addClassDescription("Copies a post-processor value into a ChainControlData.");
  return params;
}

GetPostprocessorChainControl::GetPostprocessorChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _value(declareChainControlData<Real>(getParam<PostprocessorName>("postprocessor"))),
    _pp_value(getPostprocessorValue("postprocessor"))
{
}

void
GetPostprocessorChainControl::execute()
{
  _value = _pp_value;
}
