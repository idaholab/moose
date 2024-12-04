//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LimitChainControl.h"

registerMooseObject("MooseApp", LimitChainControl);

InputParameters
LimitChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription("Limits a control value by a range.");

  params.addRequiredParam<std::string>("control_data", "Control data to limit");
  params.addParam<Real>(
      "min_value", std::numeric_limits<Real>::lowest(), "Minimum value for the control data");
  params.addParam<Real>(
      "max_value", std::numeric_limits<Real>::max(), "Maximum value for the control data");

  return params;
}

LimitChainControl::LimitChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value")),
    _unlimited_value(getChainControlData<Real>("control_data")),
    _limited_value(declareChainControlData<Real>("value"))
{
}

void
LimitChainControl::execute()
{
  _limited_value = std::max(std::min(_unlimited_value, _max_value), _min_value);
}
