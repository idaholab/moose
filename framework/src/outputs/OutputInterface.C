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
#include "OutputInterface.h"
#include "OutputWarehouse.h"
#include "MooseApp.h"
#include "ActionWarehouse.h"

// Define input parameters
template<>
InputParameters validParams<OutputInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<OutputName> >("outputs", "Vector of output names were you would like to restrict the output of variables(s) associated with this object");
  return params;
}


OutputInterface::OutputInterface(const std::string & name, InputParameters parameters, bool build_list) :
    _oi_moose_app(*parameters.get<MooseApp *>("_moose_app")),
    _oi_output_warehouse(_oi_moose_app.getOutputWarehouse()),
    _oi_outputs(parameters.get<std::vector<OutputName> >("outputs").begin(),
                parameters.get<std::vector<OutputName> >("outputs").end())
{
  // By default it is assumed that the variable name associated with 'outputs' is the name
  // of the block, this is the case for Markers, Indicators, VectorPostprocessors, and Postprocessors.
  // However, for Materials this is not the case, so the ability to call buildOutputHideVariableList
  // explicitly is needed by Materials, the build_list allows for this behavior.
  if (build_list)
  {
    std::set<std::string> names_set;
    names_set.insert(name);
    buildOutputHideVariableList(names_set);
  }
}

void
OutputInterface::buildOutputHideVariableList(std::set<std::string> variable_names)
{
  // Set of available names (a copy is used intentionally to handle the empty case discussed below)
  std::set<OutputName> avail =_oi_output_warehouse.getOutputNames();

  // If avail is empty then there is a chance that the creation of this object is happening
  // prior to the creation of the Output objects, thus extract the names from ActionWarehouse,
  // this is the case for RELAP-7 Component creation
  if (avail.empty())
  {
    ActionWarehouse & awh = _oi_moose_app.actionWarehouse();
    const std::vector<Action *> & actions = awh.getActionsByName("add_output");
    for (std::vector<Action *>::const_iterator it = actions.begin(); it != actions.end(); ++it)
      avail.insert((*it)->getShortName());
  }

  // Check for 'none'; hide variables on all outputs
  if (_oi_outputs.find("none") != _oi_outputs.end())
    for (std::set<OutputName>::const_iterator it = avail.begin(); it != avail.end(); ++ it)
      _oi_output_warehouse.addInterfaceHideVariables(*it, variable_names);

  // Check for empty and 'all' in 'outputs' parameter; do not perform any variable restrictions in these cases
  else if (_oi_outputs.empty() || _oi_outputs.find("all") != _oi_outputs.end())
    return;

  // Limit the variable output to Output objects listed
  else
  {
    // Create a list of outputs where the variable should be hidden
    std::set<OutputName> hide;
    std::set_difference(avail.begin(), avail.end(), _oi_outputs.begin(), _oi_outputs.end(), std::inserter(hide, hide.begin()));

    // If 'outputs' is specified add the object name to the list of items to hide
    for (std::set<OutputName>::const_iterator it = hide.begin(); it != hide.end(); ++ it)
      _oi_output_warehouse.addInterfaceHideVariables(*it, variable_names);
  }
}

const std::set<OutputName> &
OutputInterface::getOutputs()
{
  return _oi_outputs;
}
