//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UnitTripChainControl.h"

registerMooseObject("MooseApp", UnitTripChainControl);

InputParameters
UnitTripChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription("Trips a boolean value if an input boolean value is a certain value.");

  params.addRequiredParam<std::string>("input",
                                       "The boolean chain control data to determine if tripped");
  params.addParam<bool>("trip_on_true",
                        true,
                        "If set to 'true', the trip occurs if the input has a value of 'true'; "
                        "else the trip occurs if the input has a value of 'false'.");

  return params;
}

UnitTripChainControl::UnitTripChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _trip_on_true(getParam<bool>("trip_on_true")),
    _input(getChainControlData<bool>("input")),
    _tripped(declareChainControlData<bool>("tripped"))
{
  _tripped = false;
}

void
UnitTripChainControl::execute()
{
  if (_trip_on_true && _input)
    _tripped = true;

  if (!_trip_on_true && !_input)
    _tripped = true;
}
