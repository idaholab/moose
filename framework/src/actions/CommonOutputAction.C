//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CommonOutputAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "Output.h"
#include "OutputWarehouse.h"
#include "MooseUtils.h"

// Extrnal includes
#include "tinydir.h"
#include "pcrecpp.h"
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

registerMooseAction("MooseApp", CommonOutputAction, "common_output");
registerMooseAction("MooseApp", CommonOutputAction, "add_output");
registerMooseAction("MooseApp", CommonOutputAction, "add_reporter");

const ReporterName CommonOutputAction::perf_graph_json_reporter("perf_graph_json", "graph");

InputParameters
CommonOutputAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Adds short-cut syntax and common parameters to the Outputs block.");

  // Short-cut methods for typical output objects
  params.addParam<bool>(
      "exodus", false, "Output the results using the default settings for Exodus output.");
  params.addParam<bool>(
      "nemesis", false, "Output the results using the default settings for Nemesis output");
  params.addParam<bool>(
      "console", true, "Output the results using the default settings for Console output");
  params.addParam<bool>("csv",
                        false,
                        "Output the scalar variable and postprocessors to a *.csv "
                        "file using the default CSV output.");
  params.addParam<bool>("xml",
                        false,
                        "Output the vector postprocessors to a *.xml "
                        "file using the default XML output.");
  params.addParam<bool>("json",
                        false,
                        "Output Reporter values to a *.json "
                        "file using the default JSON output.");
  params.addParam<bool>(
      "vtk", false, "Output the results using the default settings for VTKOutput output");
  params.addParam<bool>(
      "xda", false, "Output the results using the default settings for XDA/XDR output (ascii)");
  params.addParam<bool>(
      "xdr", false, "Output the results using the default settings for XDA/XDR output (binary)");
  params.addParam<bool>(
      "gmv", false, "Output the results using the default settings for GMV output");
  params.addParam<bool>(
      "tecplot", false, "Output the results using the default settings for Tecplot output");
  params.addParam<bool>(
      "gnuplot",
      false,
      "Output the scalar and postprocessor results using the default settings for GNUPlot output");
  params.addParam<bool>(
      "solution_history", false, "Print a solution history file (.slh) using the default settings");
  params.addParam<bool>("progress", false, "Print a progress bar");
  params.addParam<bool>("dofmap", false, "Create the dof map .json output file");
  params.addParam<bool>("controls", false, "Enable the screen output of Control systems.");

  // Common parameters

  // Note: Be sure that objects that share these parameters utilize the same defaults
  params.addParam<bool>("color", true, "Set to false to turn off all coloring in all outputs");
  params.addParam<std::string>("file_base",
                               "Common file base name to be utilized with all output objects");
  params.addParam<std::vector<std::string>>("output_if_base_contains",
                                            "If this is supplied then output will only be done in "
                                            "the case that the output base contains one of these "
                                            "strings.  This is helpful in outputting only a subset "
                                            "of outputs when using MultiApps.");
  params.addParam<unsigned int>(
      "time_step_interval", 1, "The interval (number of time steps) at which output occurs");
  params.addParam<unsigned int>("interval",
                                "The interval (number of time steps) at which output occurs");
  params.deprecateParam("interval", "time_step_interval", "02/01/2025");
  params.addParam<std::vector<Real>>("sync_times",
                                     std::vector<Real>(),
                                     "Times at which the output and solution is forced to occur");
  params.addParam<Real>(
      "minimum_time_interval", 0.0, "The minimum simulation time between output steps");
  params.addParam<bool>(
      "append_date", false, "When true the date and time are appended to the output filename.");
  params.addParam<std::string>("append_date_format",
                               "The format of the date/time to append (see "
                               "http://www.cplusplus.com/reference/ctime/"
                               "strftime).");

  params.addParam<std::vector<VariableName>>(
      "hide",
      {},
      "A list of the variables and postprocessors that should NOT be output to the Exodus "
      "file (may include Variables, ScalarVariables, and Postprocessor names).");
  params.addParam<std::vector<VariableName>>(
      "show",
      {},
      "A list of the variables and postprocessors that should be output to the Exodus file "
      "(may include Variables, ScalarVariables, and Postprocessor names).");

  // Add the 'execute_on' input parameter
  ExecFlagEnum exec_enum = Output::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.addParam<ExecFlagEnum>("execute_on", exec_enum, exec_enum.getDocString());

  // Add special Console flags
  params.addDeprecatedParam<bool>(
      "print_perf_log", false, "Use perf_graph instead!", "Use perf_graph instead!");

  params.addParam<bool>(
      "perf_graph", false, "Enable printing of the performance graph to the screen (Console)");

  params.addParam<bool>("perf_graph_live", true, "Enables printing of live progress messages");
  params.addParam<Real>(
      "perf_graph_live_time_limit", 5.0, "Time (in seconds) to wait before printing a message.");
  params.addParam<unsigned int>(
      "perf_graph_live_mem_limit", 100, "Memory (in MB) to cause a message to be printed.");
  params.addParam<bool>("perf_graph_json", false, "Output the perf graph in JSON");
  params.addParam<FileName>("perf_graph_json_file", "Path to a .json file to store the perf graph");

  params.addParam<bool>("print_mesh_changed_info",
                        false,
                        "When true, each time the mesh is changed the mesh information is printed");
  params.addParam<bool>("print_linear_residuals",
                        true,
                        "Enable printing of linear residuals to the screen (Console)");
  params.addParam<bool>("print_nonlinear_residuals",
                        true,
                        "Enable printing of nonlinear residuals to the screen (Console)");
  params.addParam<bool>("print_nonlinear_converged_reason",
                        true,
                        "Enable/disable printing of the nonlinear solver convergence reason to the "
                        "screen. This parameter only affects the output of the third-party solver "
                        "(e.g. PETSc), not MOOSE itself.");
  params.addParam<bool>("print_linear_converged_reason",
                        true,
                        "Enable/disable printing of the linear solver convergence reason to the "
                        "screen. This parameter only affects the output of the third-party solver "
                        "(e.g. PETSc), not MOOSE itself.");

  params.addParam<bool>(
      "solution_invalidity_history",
      true,
      "Enable printing of the time history of the solution invalidity occurrences "
      "to the screen (console)");

  // Return object
  return params;
}

CommonOutputAction::CommonOutputAction(const InputParameters & params)
  : Action(params), _action_params(_action_factory.getValidParams("AddOutputAction"))
{
}

void
CommonOutputAction::act()
{
  if (_current_task == "common_output")
  {
    // Store the common output parameters in the OutputWarehouse
    _app.getOutputWarehouse().setCommonParameters(&_pars);

// Create the actions for the short-cut methods
#ifdef LIBMESH_HAVE_EXODUS_API
    if (getParam<bool>("exodus"))
      create("Exodus", "exodus");
#else
    if (getParam<bool>("exodus"))
      mooseWarning("Exodus output requested but not enabled through libMesh");
#endif

#ifdef LIBMESH_HAVE_NEMESIS_API
    if (getParam<bool>("nemesis"))
      create("Nemesis", "nemesis");
#else
    if (getParam<bool>("nemesis"))
      mooseWarning("Nemesis output requested but not enabled through libMesh");
#endif

    // Only create a Console if screen output was not created
    if (getParam<bool>("console") && !hasConsole())
      create("Console", "console");

    if (getParam<bool>("csv"))
      create("CSV", "csv");

    if (getParam<bool>("xml"))
      create("XMLOutput", "xml");

    if (getParam<bool>("json"))
      create("JSON", "json");

#ifdef LIBMESH_HAVE_VTK
    if (getParam<bool>("vtk"))
      create("VTK", "vtk");
#else
    if (getParam<bool>("vtk"))
      mooseWarning("VTK output requested but not enabled through libMesh");
#endif

    if (getParam<bool>("xda"))
      create("XDA", "xda");

    if (getParam<bool>("xdr"))
      create("XDR", "xdr");

    if (getParam<bool>("gmv"))
      create("GMV", "gmv");

    if (getParam<bool>("tecplot"))
      create("Tecplot", "tecplot");

    if (getParam<bool>("gnuplot"))
      create("Gnuplot", "gnuplot");

    if (getParam<bool>("solution_history"))
      create("SolutionHistory", "solution_history");

    if (getParam<bool>("progress"))
      create("Progress", "progress");

    if (getParam<bool>("dofmap"))
      create("DOFMap", "dofmap");

    // Helper for looking through a pair of [parameters, param name]
    // to find a true boolean parameter, returning the pair that was found
    const auto find_param =
        [](const std::vector<std::pair<const InputParameters *, std::string>> & options)
    {
      std::optional<std::string> from_param_name;
      const InputParameters * from_params = nullptr;
      for (const auto & [params, param_name] : options)
        if (params->template get<bool>(param_name))
        {
          from_param_name = param_name;
          from_params = params;
          break;
        }
      return std::make_pair(from_param_name, from_params);
    };

    {
      const auto [from_param_name, from_params] =
          find_param({{&parameters(), "controls"}, {&_app.parameters(), "show_controls"}});
      if (from_param_name)
        create("ControlOutput", *from_param_name, from_params);
    }

    if (!_app.getParam<bool>("no_timing"))
    {
      auto [from_param_name, from_params] = find_param({{&parameters(), "perf_graph"},
                                                        {&parameters(), "print_perf_log"},
                                                        {&_app.parameters(), "timing"}});
      if (from_param_name)
        create("PerfGraphOutput", *from_param_name, from_params);

      const auto add_perf_graph_json = [this](const std::string & from_param_name,
                                              const std::string & set_param_name,
                                              const std::string & set_param_value)
      {
        // To avoid this reporter value appearing in all other JSON output
        _common_reporter_names.push_back(perf_graph_json_reporter);

        auto params = _factory.getValidParams("JSON");
        params.set<std::vector<ReporterName>>("reporters") = {perf_graph_json_reporter};
        params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL};
        params.set<ExecFlagEnum>("execute_system_information_on") = {EXEC_NONE};
        params.set<std::string>(set_param_name) = set_param_value;
        params.set<bool>("distributed") = false;
        if (set_param_name == "file_base")
          params.set<bool>("append_date") = false;
        create("JSON", from_param_name, &parameters(), &params, from_param_name);
      };

      if (getParam<bool>("perf_graph_json"))
        add_perf_graph_json("perf_graph_json", "file_base_suffix", "perf_graph");
      if (isParamValid("perf_graph_json_file"))
      {
        const auto & file = getParam<FileName>("perf_graph_json_file");
        if (MooseUtils::getExtension(file, true) != "json")
          paramError("perf_graph_json_file", "File must end with .json");
        const auto file_stripped = MooseUtils::stripExtension(file, true);
        add_perf_graph_json("perf_graph_json_file", "file_base", file_stripped);
      }

      if (!getParam<bool>("perf_graph_live"))
      {
        if (!from_param_name)
          perfGraph().setActive(false);
        perfGraph().disableLivePrint();
      }
    }
    else
    {
      perfGraph().setActive(false);
      perfGraph().disableLivePrint();
    }

    perfGraph().setLiveTimeLimit(getParam<Real>("perf_graph_live_time_limit"));
    perfGraph().setLiveMemoryLimit(getParam<unsigned int>("perf_graph_live_mem_limit"));

    if (getParam<bool>("solution_invalidity_history"))
    {
      create("SolutionInvalidityOutput", "solution_invalidity_history");
    }

    if (!getParam<bool>("color"))
      Moose::setColorConsole(false);
  }
  else if (_current_task == "add_output")
  {
    // More like remove output. This can't be done during the "common_output" task because the
    // finite element problem is not yet constructed

    if (!getParam<bool>("console") || (isParamValid("print_nonlinear_converged_reason") &&
                                       !getParam<bool>("print_nonlinear_converged_reason")))
      Moose::PetscSupport::dontAddNonlinearConvergedReason(*_problem);

    if (!getParam<bool>("console") || (isParamValid("print_linear_converged_reason") &&
                                       !getParam<bool>("print_linear_converged_reason")))
      Moose::PetscSupport::dontAddLinearConvergedReason(*_problem);
  }
  else if (_current_task == "add_reporter")
  {
    if (!_app.getParam<bool>("no_timing") &&
        (getParam<bool>("perf_graph_json") || isParamValid("perf_graph_json_file")))
    {
      auto params = _factory.getValidParams("PerfGraphReporter");
      params.set<ExecFlagEnum>("execute_on") = EXEC_FINAL;
      _problem->addReporter("PerfGraphReporter", perf_graph_json_reporter.getObjectName(), params);
    }
  }
  else
    mooseError("unrecognized task ", _current_task, " in CommonOutputAction.");
}

void
CommonOutputAction::create(const std::string & object_type,
                           const std::optional<std::string> & param_name,
                           const InputParameters * const from_params /* = nullptr */,
                           const InputParameters * const apply_params /* = nullptr */,
                           const std::optional<std::string> & object_name /* = {} */)
{
  // Create our copy of the parameters
  auto params = _action_params;

  // Set the 'type =' parameters for the desired object
  params.set<std::string>("type") = object_type;

  // Create the complete object name (uses lower case of type)
  std::string name;
  if (object_name)
    name = *object_name;
  else
  {
    name = object_type;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  }

  // Create the action
  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create("AddOutputAction", name, params));
  auto & object_params = action->getObjectParams();

  // Set flag indicating that the object to be created was created with short-cut syntax
  object_params.set<bool>("_built_by_moose") = true;

  // Apply any additional parameters
  if (apply_params)
    object_params.applyParameters(*apply_params);

  // Associate all action output object errors with param_name
  // If from_params is specified, it means to associate it with parameters other than parameters()
  if (param_name)
  {
    const InputParameters & associated_params = from_params ? *from_params : parameters();
    associateWithParameter(associated_params, *param_name, object_params);
  }

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}

bool
CommonOutputAction::hasConsole()
{

  // Loop through all of the actions for adding output objects
  for (ActionIterator it = _awh.actionBlocksWithActionBegin("add_output");
       it != _awh.actionBlocksWithActionEnd("add_output");
       it++)
  {
    MooseObjectAction * moa = dynamic_cast<MooseObjectAction *>(*it);
    if (!moa)
      continue;

    const std::string & type = moa->getMooseObjectType();
    InputParameters & params = moa->getObjectParams();
    if (type.compare("Console") == 0 && params.get<bool>("output_screen"))
      return true;
  }

  return false;
}
