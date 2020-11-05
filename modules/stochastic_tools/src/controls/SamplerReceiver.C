//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SamplerReceiver.h"

registerMooseObjectDeprecated("StochasticToolsApp", SamplerReceiver, "01/01/2020 24:00");

InputParameters
SamplerReceiver::validParams()
{
  InputParameters params = ParameterReceiver::validParams();
  return params;
}

SamplerReceiver::SamplerReceiver(const InputParameters & parameters) : ParameterReceiver(parameters)
{
  mooseDeprecated(name(), ": use ParameterReceiver with same syntax as SamplerReceiver");
}
