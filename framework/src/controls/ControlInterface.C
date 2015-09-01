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


#include "ControlInterface.h"
#include "InputParameterWarehouse.h"
#include "MooseApp.h"

template<>
InputParameters validParams<ControlInterface>()
{
  return emptyInputParameters();
}


ControlInterface::ControlInterface(const InputParameters & parameters) :
    _ci_parameters(parameters),
    _ci_app(*parameters.get<MooseApp *>("_moose_app")),
    _input_parameter_warehouse(_ci_app.getInputParameterWarehouse()),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _ci_name(parameters.get<std::string>("name"))
{
}

std::deque<std::string>
ControlInterface::tokenizeName(const std::string & name)
{
  // Strip the parameter name into components
  std::vector<std::string> components;
  MooseUtils::tokenize(name, components);

  // Convert to deque so we can push_front
  std::deque<std::string> output(components.begin(), components.end());

  // Separate the components into variables for testing against
  // [System]
  //   [./group]
  //     item = ...
  std::size_t n = components.size();
  if (n == 1)
  {
    output.push_front("");
    output.push_front("");
  }
  else if (n == 2)
    output.push_front("");

  else if (n > 3)
    mooseError("The desired controlable parameter '" << name << "' does not match the expected naming convection.");

  // Set '*' to empty
  if (output[0] == "*")
    output[0] = "";
  if (output[1] == "*")
    output[1] = "";

  return output;
}
