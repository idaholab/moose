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

std::pair<MooseObjectName, std::string>
ControlInterface::tokenizeName(std::string name)
{
  // The MooseObject and parameter name to return
  MooseObjectName object_name;
  std::string param_name;

  // The tag precedes the :: (this is used in _moose_base::name and control_tag::name conventions)
  std::size_t idx = name.find("::");
  if (idx != std::string::npos)
  {
    object_name.tag = name.substr(0, idx);
    name.erase(0, idx+2);
  }

  // Locate the param name
  idx = name.rfind("/");
  if (idx != std::string::npos)
  {
    param_name = name.substr(idx+1);
    name.erase(idx);
  }
  else // if a slash isn't located then the entire name must be the parameter
  {
    param_name = name;
    name.erase();
  }

  // Whatever is remaining is the object name
  object_name.name = name;

  // Handle asterisks
  if (object_name.tag == "*")
    object_name.tag = "";
  if (object_name.name == "*")
    object_name.name = "";

  return std::pair<MooseObjectName, std::string>(object_name, param_name);
}
