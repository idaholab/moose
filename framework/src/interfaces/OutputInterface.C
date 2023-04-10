//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "OutputInterface.h"
#include "OutputWarehouse.h"
#include "MooseApp.h"
#include "ActionWarehouse.h"

// Define input parameters
InputParameters
OutputInterface::validParams()
{

  InputParameters params = emptyInputParameters();
  params.addClassDescription("Interface to handle the restriction of outputs from objects");
  params.addParam<std::vector<OutputName>>("outputs",
                                           "Vector of output names where you would like "
                                           "to restrict the output of variables(s) "
                                           "associated with this object");

  params.addParamNamesToGroup("outputs", "Advanced");
  std::set<std::string> reserved = {"all", "none"};
  params.setReservedValues("outputs", reserved);

  return params;
}

OutputInterface::OutputInterface(const InputParameters & parameters, bool build_list)
  : _oi_moose_app(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    _oi_output_warehouse(_oi_moose_app.getOutputWarehouse()),
    _oi_outputs(parameters.get<std::vector<OutputName>>("outputs").begin(),
                parameters.get<std::vector<OutputName>>("outputs").end())
{

  // By default it is assumed that the variable name associated with 'outputs' is the name
  // of the block, this is the case for Markers, Indicators, VectorPostprocessors, and
  // Postprocessors.
  // However, for Materials this is not the case, so the call to buildOutputHideVariableList must be
  // disabled, the build_list allows for this behavior. The hide lists are handled by
  // MaterialOutputAction in this case.
  //
  // Variables/AuxVariables also call the buildOutputHideVariableList method later, because when
  // their actions are called the Output objects do not exist. This case is handled by the
  // CheckOutputAction::checkVariableOutput.
  if (build_list)
  {
    std::set<std::string> names_set;
    names_set.insert(parameters.get<std::string>("_object_name"));
    buildOutputHideVariableList(names_set);
  }
}

void
OutputInterface::buildOutputHideVariableList(std::set<std::string> variable_names)
{
  // Set of available names
  const std::set<OutputName> & avail = _oi_output_warehouse.getOutputNames();

  // Check for 'none'; hide variables on all outputs
  if (_oi_outputs.find("none") != _oi_outputs.end())
    for (const auto & name : avail)
      _oi_output_warehouse.addInterfaceHideVariables(name, variable_names);

  // Check for empty and 'all' in 'outputs' parameter; do not perform any variable restrictions in
  // these cases
  else if (_oi_outputs.empty() || _oi_outputs.find("all") != _oi_outputs.end())
    return;

  // Limit the variable output to Output objects listed
  else
  {
    // Create a list of outputs where the variable should be hidden
    std::set<OutputName> hide;
    std::set_difference(avail.begin(),
                        avail.end(),
                        _oi_outputs.begin(),
                        _oi_outputs.end(),
                        std::inserter(hide, hide.begin()));

    // If 'outputs' is specified add the object name to the list of items to hide
    for (const auto & name : hide)
      _oi_output_warehouse.addInterfaceHideVariables(name, variable_names);
  }
}

const std::set<OutputName> &
OutputInterface::getOutputs()
{
  return _oi_outputs;
}
