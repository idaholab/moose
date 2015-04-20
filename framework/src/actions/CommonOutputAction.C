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
#include "CommonOutputAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "Output.h"
#include "OutputWarehouse.h"

// Extrnal includes
#include "tinydir.h"
#include "pcrecpp.h"
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

template<>
InputParameters validParams<CommonOutputAction>()
{
   InputParameters params = validParams<Action>();

   // Short-cut methods for typical output objects
   params.addParam<bool>("exodus", false, "Output the results using the default settings for Exodus output");
   params.addParam<bool>("nemesis", false, "Output the results using the default settings for Nemesis output");
   params.addParam<bool>("console", true, "Output the results using the default settings for Console output");
   params.addParam<bool>("csv", false, "Output the scalar variable and postprocessors to a *.csv file using the default CSV output.");
   params.addParam<bool>("vtk", false, "Output the results using the default settings for VTKOutput output");
   params.addParam<bool>("xda", false, "Output the results using the default settings for XDA/XDR output (ascii)");
   params.addParam<bool>("xdr", false, "Output the results using the default settings for XDA/XDR output (binary)");
   params.addParam<bool>("checkpoint", false, "Create checkpoint files using the default options.");
   params.addParam<bool>("gmv", false, "Output the results using the default settings for GMV output");
   params.addParam<bool>("tecplot", false, "Output the results using the default settings for Tecplot output");
   params.addParam<bool>("gnuplot", false, "Output the scalar and postprocessor results using the default settings for GNUPlot output");
   params.addParam<bool>("solution_history", false, "Print a solution history file (.slh) using the default settings");
   params.addParam<bool>("dofmap", false, "Create the dof map .json output file");

   // Common parameters

   // Note: Be sure that objects that share these parameters utilize the same defaults
   params.addParam<bool>("color", true, "Set to false to turn off all coloring in all outputs");
   params.addParam<std::string>("file_base", "Common file base name to be utilized with all output objects");
   params.addParam<std::vector<std::string> >("output_if_base_contains", "If this is supplied then output will only be done in the case that the output base contains one of these strings.  This is helpful in outputting only a subset of outputs when using MultiApps.");
   params.addParam<unsigned int>("interval", 1, "The interval at which timesteps are output to the solution file");
   params.addParam<std::vector<Real> >("sync_times", std::vector<Real>(), "Times at which the output and solution is forced to occur");

   params.addParam<std::vector<VariableName> >("hide", "A list of the variables and postprocessors that should NOT be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");
   params.addParam<std::vector<VariableName> >("show", "A list of the variables and postprocessors that should be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");

  // Add the 'output_on' input parameter
  params.addParam<MultiMooseEnum>("output_on", Output::getExecuteOptions("timestep_end"), "Set to (initial|linear|nonlinear|timestep_end|timestep_begin|final|failed|custom) to execute only at that moment (default: timestep_end)");

  // Add common output toggles
  params.addParam<bool>("output_initial", false, "Request that the initial condition is output to the solution file");
  params.addParam<bool>("output_timestep_end", true, "Request that data be output at the end of the timestep");
  params.addDeprecatedParam<bool>("output_intermediate", true, "Request that all intermediate steps (not initial or final) are output",
                                  "replace by adding 'timestep_end' to the 'output_on' option");
  params.addParam<bool>("output_final", false, "Force the final time step to be output, regardless of output interval");

  // Add special Console flags
  params.addParam<bool>("print_linear_residuals", false, "Enable printing of linear residuals to the screen (Console)");
  params.addParam<bool>("print_perf_log", false, "Enable printing of the performance log to the screen (Console)");
  params.addParam<bool>("print_mesh_changed_info", false, "When true, each time the mesh is changed the mesh information is printed");

  // Return object
  return params;
}

CommonOutputAction::CommonOutputAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _action_params(_action_factory.getValidParams("AddOutputAction"))
{
  // Set the ActionWarehouse pointer in the parameters that will be passed to the actions created with this action
  _action_params.set<ActionWarehouse *>("awh") = &_awh;

  // Support quick output toggles
  MultiMooseEnum & output_on = _pars.set<MultiMooseEnum>("output_on");
  if (getParam<bool>("output_initial"))
    output_on.push_back("initial");
  if (getParam<bool>("output_timestep_end"))
    output_on.push_back("timestep_end");
  if (getParam<bool>("output_final"))
    output_on.push_back("final");

  // **** DEPRECATED PARAMETER SUPPORT ****
  if (getParam<bool>("output_intermediate"))
    output_on.push_back("timestep_end");
}

void
CommonOutputAction::act()
{
  // Store the common output parameters in the OutputWarehouse
  _app.getOutputWarehouse().setCommonParameters(&_pars);

  // Create the actions for the short-cut methods
#ifdef LIBMESH_HAVE_EXODUS_API
  if (getParam<bool>("exodus"))
    create("Exodus");
#else
  if (getParam<bool>("exodus"))
    mooseWarning("Exodus output requested but not enabled through libMesh");
#endif

#ifdef LIBMESH_HAVE_NEMESIS_API
  if (getParam<bool>("nemesis"))
    create("Nemesis");
#else
  if (getParam<bool>("nemesis"))
    mooseWarning("Nemesis output requested but not enabled through libMesh");
#endif

  // Only create a Console if screen output was not created
  if (getParam<bool>("console") && !hasConsole())
    create("Console");
  else
    _pars.set<bool>("console") = false;

  if (getParam<bool>("csv"))
    create("CSV");

#ifdef LIBMESH_HAVE_VTK
  if (getParam<bool>("vtk"))
    create("VTK");
#else
  if (getParam<bool>("vtk"))
    mooseWarning("VTK output requested but not enabled through libMesh");
#endif

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
    create("Gnuplot");

  if (getParam<bool>("solution_history"))
    create("SolutionHistory");

  if (getParam<bool>("dofmap"))
    create("DOFMap");

  if (!getParam<bool>("color"))
    Moose::_color_console = false;
}

void
CommonOutputAction::create(std::string object_type)
{
  // Set the 'type =' parameters for the desired object
  _action_params.set<std::string>("type") = object_type;

  // Create the complete object name (uses lower case of type)
  std::transform(object_type.begin(), object_type.end(), object_type.begin(), ::tolower);
  std::string long_name("Outputs/");
  long_name += object_type;

  // Create the action
  MooseSharedPointer<MooseObjectAction> action = MooseSharedNamespace::static_pointer_cast<MooseObjectAction>(_action_factory.create("AddOutputAction", long_name, _action_params));

  // Set flag indicating that the object to be created was created with short-cut syntax
  action->getObjectParams().set<bool>("_built_by_moose") = true;

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}

bool
CommonOutputAction::hasConsole()
{

  // Loop through all of the actions for adding output objects
  for (ActionIterator it = _awh.actionBlocksWithActionBegin("add_output"); it != _awh.actionBlocksWithActionEnd("add_output"); it++)
  {
    MooseObjectAction * moa = static_cast<MooseObjectAction *>(*it);
    const std::string & type = moa->getMooseObjectType();
    InputParameters & params = moa->getObjectParams();
    if (type.compare("Console") == 0 && params.get<bool>("output_screen"))
      return true;
  }

  return false;
}
