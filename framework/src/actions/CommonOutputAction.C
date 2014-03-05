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

#include "CommonOutputAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "Exodus.h"
#include "OutputWarehouse.h"

template<>
InputParameters validParams<CommonOutputAction>()
{
   InputParameters params = validParams<Action>();

   // Short-cut methods for typical output objects
   params.addParam<bool>("exodus", false, "Output the results using the default settings for Exodus output");
   params.addParam<bool>("nemesis", false, "Output the results using the default settings for Nemesis output");
   params.addParam<bool>("console", false, "Output the results using the default settings for Console output");
   params.addParam<bool>("csv", false, "Output the scalar variable and postprocessors to a *.csv file using the default CSV output.");
   params.addParam<bool>("vtk", false, "Output the results using the default settings for VTK output");
   params.addParam<bool>("xda", false, "Output the results using the default settings for XDA/XDR output (ascii)");
   params.addParam<bool>("xdr", false, "Output the results using the default settings for XDA/XDR output (binary)");
   params.addParam<bool>("checkpoint", false, "Create checkpoint files using the default options.");
   params.addParam<bool>("gmv", false, "Output the results using the default settings for GMV output");
   params.addParam<bool>("tecplot", false, "Output the results using the default settings for Tecplot output");
   params.addParam<bool>("gnuplot", false, "Output the scalar and postprocessor results using the default settings for GNUPlot output");

   // Common parameters
   params.addParam<bool>("output_initial", false,  "Request that the initial condition is output to the solution file");
   params.addParam<bool>("output_final", false, "Force the final timestep to be output, regardless of output interval");

   params.addParam<std::string>("file_base", "Common file base name to be utilized with all output objects");
   params.addParam<unsigned int>("interval", 1, "The interval at which timesteps are output to the solution file");
   params.addParam<std::vector<Real> >("sync_times", "Times at which the output and solution is forced to occur");

   params.addParam<std::vector<VariableName> >("hide", "A list of the variables and postprocessors that should NOT be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");
   params.addParam<std::vector<VariableName> >("show", "A list of the variables and postprocessors that should be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");

   return params;

}

CommonOutputAction::CommonOutputAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _action_params(_action_factory.getValidParams("AddOutputAction"))
{
  // Set the ActionWarehouse pointer in the parameters that will be passed to the actions created with this action
  _action_params.set<ActionWarehouse *>("awh") = &_awh;
}

void
CommonOutputAction::act()
{
  // Store the common output parameters in the OutputWarehouse
  _app.getOutputWarehouse().setCommonParameters(&_pars);

  // Create the actions for the short-cut methods
  if (getParam<bool>("exodus"))
    create("Exodus");

  if (getParam<bool>("nemesis"))
    create("Nemesis");

  if (getParam<bool>("console"))
    create("Console");

  if (getParam<bool>("csv"))
    create("CSV");

  if (getParam<bool>("vtk"))
    create("VTK");

  if (getParam<bool>("xda"))
    create("XDA");

  if (getParam<bool>("xdr"))
    create("XDR");

  if (getParam<bool>("checkpoint"))
    create("Checkpoint");

  if (getParam<bool>("gmv"))
    create("GMV");

  if (getParam<bool>("tecplot"))
    create("Tecplot");

  if (getParam<bool>("gnuplot"))
    create("GNUPlot");

}

void
CommonOutputAction::create(std::string object_type)
{
  // Set the 'type =' parameters for the desired object
  _action_params.set<std::string>("type") = object_type;

  // Create the complete object name (use lower case for the object name)
  std::transform(object_type.begin(), object_type.end(), object_type.begin(), ::tolower);
  std::string long_name("Outputs/");
  long_name += object_type;

  // Create the action
  MooseObjectAction * action = static_cast<MooseObjectAction *>(_action_factory.create("AddOutputAction", long_name, _action_params));

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}
