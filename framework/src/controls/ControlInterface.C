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
  if (libMesh::n_threads() > 1)
    mooseError("The control logic system is experimental and under heavy development, it currently does not work with threading.");
}

ControlParameterName
ControlInterface::tokenizeName(const std::string & name)
{
  ControlParameterName output;

  // Strip the parameter name into components
  std::vector<std::string> components;
  MooseUtils::tokenize(name, components);

  // Separate the components into variables for testing against
  // [System]
  //   [./object]
  //     param = ...
  std::size_t n = components.size();
  if (n == 1)
    output.param = components[0];

  else if (n == 2)
  {
    output.object = components[0];
    output.param = components[1];
  }

  else if (n == 3)
  {
    output.system = components[0];
    output.object = components[1];
    output.param = components[2];
  }

  else if (n > 3)
    mooseError("The desired controllable parameter '" << name << "' does not match the expected naming convection.");

  // Set '*' to empty
  if (output.system == "*")
    output.system = "";
  if (output.object == "*")
    output.object = "";

  return output;
}
