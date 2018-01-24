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
#include "Function.h"

template <>
InputParameters
validParams<SamplerReceiver>()
{
  InputParameters params = validParams<Control>();
  params.addClassDescription("Control for receiving data from a Sampler via SamplerTransfer.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  return params;
}

SamplerReceiver::SamplerReceiver(const InputParameters & parameters) : Control(parameters) {}

void
SamplerReceiver::execute()
{
  for (auto & param_pair : _parameters)
    setControllableValueByName<Real>(param_pair.first, param_pair.second);
}

void
SamplerReceiver::reset()
{
  _parameters.clear();
}

void
SamplerReceiver::addControlParameter(const std::string & name, const Real & value)
{
  _parameters[name] = value;
}
