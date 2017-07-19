/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "SamplerReceiver.h"
#include "Function.h"

template <>
InputParameters
validParams<SamplerReceiver>()
{
  InputParameters params = validParams<Control>();
  params.addClassDescription("Control for receiving data from a Sampler via SamplerTransfer.");
  params.set<MultiMooseEnum>("execute_on") = "timestep_begin";
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
