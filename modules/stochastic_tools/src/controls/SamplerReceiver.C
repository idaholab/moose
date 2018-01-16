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
  std::size_t value_position = 0;

  // Loop through all the parameters and set the controllable values for each parameter.
  for (const std::string & param_name : _parameters)
  {
    // Set Real parameter
    {
      ControllableParameter<Real> control_param = getControllableParameterByName<Real>(param_name);
      if (!control_param.empty())
      {
        control_param.set(_values[value_position++]);
        continue; // continue to the next parameter
      }
    }

    // Set std::vector<Real> parameter
    {
      ControllableParameter<std::vector<Real>> control_param =
          getControllableParameterByName<std::vector<Real>>(param_name);
      if (!control_param.empty())
      {
        std::size_t n = control_param.get()[0]->size(); // size of vector to be changed

        // All vectors being controlled must be the same size
        for (const std::vector<Real> * ptr : control_param.get())
          if (ptr->size() != n)
            mooseError(
                "The std::vector<Real> parameters beging controlled must all be the same size:\n",
                control_param.dump());

        // There must be enough data to populate the controlled parameter
        if (value_position + n > _values.size())
          mooseError("The supplied vector of Real values is not sized correctly, the "
                     "std::vector<Real> parameter '",
                     param_name,
                     " requires ",
                     n,
                     " values but only ",
                     _values.size() - value_position,
                     " are available in the supplied vector.");

        // Set the value
        std::vector<Real> value(_values.begin() + value_position,
                                _values.begin() + value_position + n);
        value_position += n;
        control_param.set(value);
        continue; // continue to the next parameter
      }
    }

    // If the loop gets here it failed to find what it was looking for
    mooseError(
        "Unable to locate a Real or std::vector<Real> parameter with the name '", param_name, ".'");
  }

  // Error if there is un-used values
  if (value_position != _values.size())
    mooseError("The number of values supplied (",
               _values.size(),
               ") does not match the number of values consumed by setting parameter values (",
               value_position,
               ").");
}

void
SamplerReceiver::transfer(const std::vector<std::string> & names, const std::vector<Real> & values)
{
  _parameters = names;
  _values = values;
}
