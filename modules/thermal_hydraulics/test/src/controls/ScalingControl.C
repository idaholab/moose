//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalingControl.h"

registerMooseObject("ThermalHydraulicsTestApp", ScalingControl);

InputParameters
ScalingControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<Real>("scale", "Scaling factor");
  params.addRequiredParam<Real>("initial", "Initial value");
  params.addClassDescription("Control that multiplies old value by a scalar. Used for testing time "
                             "dependent control values.");
  return params;
}

ScalingControl::ScalingControl(const InputParameters & parameters)
  : THMControl(parameters),
    _scale(getParam<Real>("scale")),
    _initial(getParam<Real>("initial")),
    _value(declareComponentControlData<Real>("value")),
    _value_old(getComponentControlDataOld<Real>("value"))
{
  _value = _initial;
}

void
ScalingControl::execute()
{
  _value = _scale * _value_old;
}
