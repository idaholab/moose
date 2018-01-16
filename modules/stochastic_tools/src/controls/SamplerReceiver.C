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

registerMooseObject("StochasticToolsApp", SamplerReceiver);

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
  for (std::size_t i = 0; i < _parameters.size(); ++i)
    setControllableValueByName<Real>(_parameters[i], _values[i]);
}

void
SamplerReceiver::transfer(const std::vector<std::string> & names, const std::vector<Real> & values)
{
  _parameters = names;
  _values = values;
}
