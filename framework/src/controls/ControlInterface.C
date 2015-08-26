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
ControlInterface::tokenizeName(std::string name)
{

  // Create the storage container that will be output
  ControlParameterName container;

  // Locate the group name (this can be the "_moose_base" or "control_tag" parameters from the object
  std::size_t idx = name.find("::");
  if (idx != std::string::npos)
  {
    container.group = name.substr(0, idx);
    name.erase(0, idx+2);
  }

  // Locate the param name
  idx = name.rfind("/");
  if (idx != std::string::npos)
  {
    container.param = name.substr(idx+1);
    name.erase(idx);
  }
  else // if a slash isn't located then the entire name must be the parameter
  {
    container.param = name;
    name.erase();
  }

  // Locate the syntax
  idx = name.rfind("/");
  if (idx != std::string::npos)
  {
    container.syntax = name.substr(0, idx);
    name.erase(0, idx+1);
  }

  // Whatever is remaining is the object name
  container.object = name;

  // Handle asterisks
  if (container.group == "*")
    container.group = "";
  if (container.syntax == "*")
    container.syntax = "";
  if (container.object == "*")
    container.object = "";

  return container;
}
