//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScaleOldChainControl.h"

registerMooseObject("MooseApp", ScaleOldChainControl);

InputParameters
ScaleOldChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription("Scales an old control value by another control value.");

  params.addParam<std::string>("control_data",
                               "Control data whose old value is to be scaled. If no name is "
                               "provided, this control data is used.");
  params.addRequiredParam<std::string>("scale_factor", "Control data by which to scale value");
  params.addRequiredParam<Real>("initial_value", "Initial value if scaling this control data");

  return params;
}

ScaleOldChainControl::ScaleOldChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _value(declareChainControlData<Real>("value")),
    _value_old(getChainControlDataOldByName<Real>(isParamValid("control_data")
                                                      ? getParam<std::string>("control_data")
                                                      : fullControlDataName("value"))),
    _scale_factor(getChainControlData<Real>("scale_factor"))
{
  _value = getParam<Real>("initial_value");
}

void
ScaleOldChainControl::execute()
{
  _value = _scale_factor * _value_old;
}
