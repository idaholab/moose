//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SamplerReceiver2.h"
#include "Function.h"

registerMooseObject("isopodApp", SamplerReceiver2);

InputParameters
SamplerReceiver2::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription(
      "Control for receiving data from a Sampler via SamplerParameterTransfer.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  return params;
}

SamplerReceiver2::SamplerReceiver2(const InputParameters & parameters) : Control(parameters) {}

void
SamplerReceiver2::execute()
{
  std::size_t value_position = 0;

  // Loop through all the parameters and set the controllable values for each parameter.
  for (const std::string & param_name : _parameters)
  {
    ControllableParameter control_param = getControllableParameterByName(param_name);

    // Real
    if (control_param.check<Real>())
    {
      // There must be enough data to populate the controlled parameter
      if (value_position >= _values.size())
        mooseError("The supplied vector of Real values is not sized correctly, the "
                   "Real parameter '",
                   param_name,
                   " requires a value but no more values are available in "
                   "the supplied values which have a size of ",
                   _values.size(),
                   ".");
      control_param.set<Real>(_values[value_position++]);
    }

    else if (control_param.check<std::vector<Real>>())
    {
      std::vector<std::vector<Real>> values = control_param.get<std::vector<Real>>();
      mooseAssert(values.size() != 0,
                  "ControllableParameter must not be empty."); // should not be possible
      std::size_t n = values[0].size();                        // size of vector to changed

      // All vectors being controlled must be the same size
      for (const std::vector<Real> & value : values)
        if (value.size() != n)
          mooseError(
              "The std::vector<Real> parameters being controlled must all be the same size:\n",
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
      control_param.set<std::vector<Real>>(value);
    }

    else

      // If the loop gets here it failed to find what it was looking for
      mooseError("Unable to locate a Real or std::vector<Real> parameter with the name '",
                 param_name,
                 ".'");
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
SamplerReceiver2::transfer(const std::vector<std::string> & names, const std::vector<Real> & values)
{
  _parameters = names;
  _values = values;
}
