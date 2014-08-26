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

// Define input parameters
template<>
InputParameters validParams<OutputInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<OutputName> >("outputs", "Vector of output names were you would like to restrict the output of variables(s) associated with this object");
  return params;
}


OutputInterface::OutputInterface(const std::string & name, InputParameters parameters) :
    OutputInterface(name, parameters, std::vector<std::string>(1, name))
{
}

OutputInterface::OutputInterface(const std::string & name, InputParameters parameters, std::string variable_name) :
    OutputInterface(name, parameters, std::vector<std::string>(1, variable_name))
{
}

OutputInterface::OutputInterface(const std::string & name, InputParameters parameters, std::vector<std::string> variable_names) :
    _oi_moose_app(*parameters.get<MooseApp *>("_moose_app")),
    _oi_output_warehouse(_oi_moose_app.getOutputWarehouse()),
    _oi_outputs(parameters.get<std::vector<OutputName> >("outputs").begin(),
                parameters.get<std::vector<OutputName> >("outputs").end())
{
  // Extract all the possible
  const std::set<OutputName> & avail = _oi_output_warehouse.getOutputNames();

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
    // Check that the supplied outputs are valid
    _oi_output_warehouse.checkOutputs(_oi_outputs);

    // Create a list of outputs where the variable should be hidden
    std::set<OutputName> hide;
    std::set_difference(avail.begin(), avail.end(), _oi_outputs.begin(), _oi_outputs.end(), std::inserter(hide, hide.begin()));

    // If 'outputs' is specified add the object name to the list of items to hide
    for (std::set<OutputName>::const_iterator it = hide.begin(); it != hide.end(); ++ it)
      _oi_output_warehouse.addInterfaceHideVariables(*it, variable_names);
  }
}
