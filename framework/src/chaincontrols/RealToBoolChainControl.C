//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RealToBoolChainControl.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", RealToBoolChainControl);

InputParameters
RealToBoolChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription("Converts a Real-valued chain control data to boolean.");

  params.addRequiredParam<std::string>("input",
                                       "The Real-valued chain control data to convert to boolean.");

  return params;
}

RealToBoolChainControl::RealToBoolChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _input(getChainControlData<Real>("input")),
    _output(declareChainControlData<bool>("value"))
{
}

void
RealToBoolChainControl::execute()
{
  if (MooseUtils::absoluteFuzzyEqual(_input, 1.0))
    _output = true;
  else if (MooseUtils::absoluteFuzzyEqual(_input, 0.0))
    _output = false;
  else
    mooseError("The current input value (", _input, ") is not equal to 1 or 0.");
}
