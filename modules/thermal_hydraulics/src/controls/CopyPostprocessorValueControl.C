//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CopyPostprocessorValueControl.h"

registerMooseObject("ThermalHydraulicsApp", CopyPostprocessorValueControl);

InputParameters
CopyPostprocessorValueControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor.");
  return params;
}

CopyPostprocessorValueControl::CopyPostprocessorValueControl(const InputParameters & parameters)
  : THMControl(parameters),
    _value(declareControlData<Real>(getParam<PostprocessorName>("postprocessor"))),
    _pps_value(getPostprocessorValue("postprocessor"))
{
}

void
CopyPostprocessorValueControl::execute()
{
  _value = _pps_value;
}
