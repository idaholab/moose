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

InputParameters
SamplerReceiver::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription(
      "Control for receiving data from a Sampler via SamplerParameterTransfer.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  return params;
}

SamplerReceiver::SamplerReceiver(const InputParameters & parameters) : Control(parameters) {}

void
SamplerReceiver::execute()
{
  // Send parameters from root
  _communicator.broadcast(_parameters);
  _communicator.broadcast(_values);
  if (_parameters.size() != _values.size())
    mooseError("Internal error: Number of parameters does not match number of values.");

  // Loop through all the parameters and set the controllable values for each parameter.
  for (const auto & i : index_range(_parameters))
  {
    const std::string & param_name = _parameters[i];
    const std::vector<Real> & value = _values[i];

    ControllableParameter control_param = getControllableParameterByName(param_name);

    // Real
    if (control_param.check<Real>())
    {
      // There must be enough data to populate the controlled parameter
      if (value.size() != 1)
        mooseError("The Real parameter '",
                   param_name,
                   "' expects a single value, but the vector supplying its values has a size of ",
                   value.size(),
                   ".");
      control_param.set<Real>(value[0]);
    }
    else if (control_param.check<std::vector<Real>>())
      control_param.set<std::vector<Real>>(value);
    else
      // If the loop gets here it failed to find what it was looking for
      mooseError("Unable to locate a Real or std::vector<Real> parameter with the name '",
                 param_name,
                 ".'");
  }
}

void
SamplerReceiver::transfer(const std::map<std::string, std::vector<Real>> & param_values)
{
  // We'll only transfer on the root processor and gather later
  if (processor_id() == 0)
  {
    _parameters.clear();
    _values.clear();
    for (const auto & pv : param_values)
    {
      _parameters.push_back(pv.first);
      _values.push_back(pv.second);
    }
  }
}
