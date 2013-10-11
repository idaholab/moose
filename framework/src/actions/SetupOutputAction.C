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

#include "SetupOutputAction.h"

#include "MooseApp.h"
#include "Output.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "OutputProblem.h"
#include "Executioner.h"
#include "MooseMesh.h"
#include "libmesh/exodusII_io.h"

#include "unistd.h"

template<>
InputParameters validParams<SetupOutputAction>()
{
  InputParameters params = validParams<Action>();
  MooseEnum pps_fit_mode(FormattedTable::getWidthModes());

  params.addParam<OutFileBase>("file_base", "The desired solution output name without an extension (Defaults to the mesh file name + '_out' or 'out' if generating the mesh by some other means)");
  params.addParam<unsigned int>("interval", 1, "The interval at which timesteps are output to the solution file");
  params.addParam<unsigned int>("screen_interval", 1, "The interval at which postprocessors are output to the screen. This value must evenly divide \"interval\" so that postprocessors are calculated at corresponding solution timesteps. In addition, if \"screen_interval\" is strictly greater than \"interval\", \"output_initial\" must be set to true");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("nemesis", false, "Specifies that you would like Nemesis output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("vtk", false, "Specifies that you would like VTK output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("tecplot_binary", false, "Specifies that you would like Tecplot binary output solution files(s)");
  params.addParam<bool>("xda", false, "Specifies that you would like xda output solution files(s)");
  params.addParam<bool>("xdr", false, "Specifies that you would like xdr (binary) output solution file(s)");
  params.addParam<bool>("postprocessor_screen", true, "Specifies that you would like PostProcessor output to the screen (stdout)");
  params.addParam<unsigned int>("max_pps_rows_screen", 15, "The maximum number of postprocessor values displayed on screen during a timestep (set to 0 for unlimited)");
  params.addParam<MooseEnum>("pps_fit_to_screen", pps_fit_mode, "Specifies the wrapping mode for post-processor tables that are printed to the screen "
                             "(ENVIRONMENT: Read \"PPS_WIDTH\" for desired width, AUTO: Attempt to determine width automatically (serial only), <n>: Desired width");
  params.addParam<bool>("postprocessor_csv", false, "Specifies that you would like a PostProcessor comma separated values file");
  params.addParam<bool>("postprocessor_gnuplot", false, "Specifies that you would like plots of the postprocessor output");
  params.addParam<std::string>("gnuplot_format", "ps", "Specifies which output format gnuplot will produce. Currently supported: ps, gif, and png");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  params.addParam<bool>("output_displaced", false, "Requests that displaced mesh files are written at each solve");
  params.addParam<bool>("output_solution_history", false, "Requests that the 'solution history' is output, the solution history is the number of nonlinear / linear solves that are done for each step.");
  params.addParam<bool>("output_es_info", true, "Requests that we output Equation Systems info during calls to initialSetup (normallly at the beginning of a simulation.)");
  params.addParam<Real>("time_interval", "simulation time at which to solve and output");

  params.addParam<bool>("color_output", true, "If true then MOOSE will attempt to add color when it outputs to the console.");

  params.addParam<Real>("iteration_plot_start_time", std::numeric_limits<Real>::max(), "Specifies a time after which the solution will be written following each nonlinear iteration");

  params.addParam<bool>("perf_log",        false,    "Specifies whether or not the Performance log should be printed");
  params.addParam<bool>("show_setup_log_early", false, "Specifies whether or not the Setup Performance log should be printed before the first time step.  It will still be printed at the end if ""perf_log"" is also enabled and likewise disabled in ""perf_log"" is false");
  params.addParam<std::vector<VariableName> >("output_variables", "A list of the variables that should be output to the Exodus file.  If this is not provided then all variables will be in the output.");
  params.addParam<std::vector<VariableName> >("hidden_variables", "A list of the variables that should NOT be output to the Exodus file.  If this is not provided then all variables will be in the output.");
  params.addParam<bool>("elemental_as_nodal", false, "Output elemental variables also as nodal");
  params.addParam<bool>("exodus_inputfile_output", true, "Determines whether or not the input file is output to exodus - default (true)");
  params.addParam<std::vector<std::string> >("output_if_base_contains", "If this is supplied then output will only be done in the case that the output base contains one of these strings.  This is helpful in outputing only a subset of outputs when using MultiApps.");

  // restart options
  params.addParam<unsigned int>("num_restart_files", 0, "Number of the restart files to save (0 = no restart files)");
  params.addParam<Point>("position", "Set a positional offset.  This vector will get added to the nodal cooardinates to move the domain.");
  params.addParam<bool>("all_var_norms", false, "If true then all variable norms will be printed after each solve");

  params.addParamNamesToGroup("position interval time_interval output_displaced output_solution_history iteration_plot_start_time elemental_as_nodal exodus_inputfile_output output_es_info output_variables hidden_variables", "Advanced");
  params.addParamNamesToGroup("nemesis gmv vtk tecplot tecplot_binary xda xdr", "Format");
  params.addParamNamesToGroup("screen_interval postprocessor_screen max_pps_rows_screen pps_fit_to_screen postprocessor_csv postprocessor_gnuplot gnuplot_format", "Postprocessor");
  params.addParamNamesToGroup("perf_log show_setup_log_early", "Logging");
  params.addParamNamesToGroup("num_restart_files", "Restart");


  return params;
}


SetupOutputAction::SetupOutputAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupOutputAction::setupOutputObject(Output &output, InputParameters & params)
{
  mooseAssert(params.have_parameter<std::vector<VariableName> >("output_variables"), "Output Variables are required");

  OutFileBase base = params.get<OutFileBase>("file_base");

  if(params.isParamValid("output_if_base_contains"))
  {
    const std::vector<std::string> & strings = params.get<std::vector<std::string> >("output_if_base_contains");

    bool found_it = false;
    for(unsigned int i=0; i<strings.size(); i++)
      found_it = found_it || ( base.find(strings[i]) != std::string::npos);

    if(!found_it) // Didn't find a match so no output should be done
      return;
  }

  output.fileBase(base);

  if (params.get<bool>("exodus"))
  {
    if (params.have_parameter<bool>("exodus_inputfile_output") && !params.get<bool>("exodus_inputfile_output"))
      output.add(Output::EXODUS, false);
    else
      output.add(Output::EXODUS, true);
  }
  if (params.get<bool>("nemesis")) output.add(Output::NEMESIS);
  if (params.get<bool>("gmv")) output.add(Output::GMV);
  if (params.get<bool>("vtk")) output.add(Output::VTK);
  if (params.get<bool>("tecplot")) output.add(Output::TECPLOT);
  if (params.get<bool>("tecplot_binary")) output.add(Output::TECPLOT_BIN);
  if (params.get<bool>("xda")) output.add(Output::XDA);
  if (params.get<bool>("xdr")) output.add(Output::XDR);
}

void
SetupOutputAction::act()
{
  // Disable Perf Log if requested
  if (!getParam<bool>("perf_log"))
  {
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
  }

  mooseAssert(_problem, "This should never happen!");

  /// Determines whether we see the perf log early in a run or not
  _problem->setEarlyPerfLogPrint(getParam<bool>("show_setup_log_early"));

  if (_pars.isParamValid("hidden_variables"))
  {
    _problem->hideVariableFromOutput(getParam<std::vector<VariableName> >("hidden_variables"));
  }

  if (_pars.isParamValid("output_variables"))
  {
    _problem->showVariableInOutput(getParam<std::vector<VariableName> >("output_variables"));
  }
  else
  {
    if (getParam<bool>("elemental_as_nodal"))
    {
      // output all variables in the system
      _problem->showVariableInOutput(_problem->getVariableNames());
    }
  }

  if(_pars.isParamValid("position"))
    _app.setOutputPosition(_pars.get<Point>("position"));

  // If the user didn't provide a filename - see if the parser has a filename that we can use as a base
  if (!_pars.isParamValid("file_base"))
    mooseError("\"file_base\" wasn't populated either by the input file or the parser.");

  _problem->setOutputVariables();

  Output & output = _problem->out();                       // can't use use this with coupled problems on different meshes

  // Has the filebase been overriden at the application level?
  if(_app.getOutputFileBase() != "")
    _pars.set<OutFileBase>("file_base") = _app.getOutputFileBase();

  setupOutputObject(output, _pars);

  const bool output_initial = getParam<bool>("output_initial");
  if (_executioner != NULL)
    _executioner->outputInitial(output_initial);

  // TODO: handle this thru Problem interface
  _problem->_postprocessor_screen_output = getParam<bool>("postprocessor_screen");
  _problem->_postprocessor_csv_output = getParam<bool>("postprocessor_csv");
  _problem->_postprocessor_gnuplot_output = getParam<bool>("postprocessor_gnuplot");
  _problem->_gnuplot_format = getParam<std::string>("gnuplot_format");
  _problem->setMaxPPSRowsScreen(getParam<unsigned int>("max_pps_rows_screen"));
  _problem->setPPSFitScreen(getParam<MooseEnum>("pps_fit_to_screen"));

  _problem->outputDisplaced(getParam<bool>("output_displaced"));
  _problem->outputSolutionHistory(getParam<bool>("output_solution_history"));
  _problem->outputESInfo(getParam<bool>("output_es_info"));

  _problem->setNumRestartFiles(getParam<unsigned int>("num_restart_files"));


#ifdef LIBMESH_ENABLE_AMR
  Adaptivity & adapt = _problem->adaptivity();
  if (adapt.isOn())
    output.sequence(true);
#endif //LIBMESH_ENABLE_AMR

  const unsigned int interval = getParam<unsigned int>("interval");
  const unsigned int screen_interval = getParam<unsigned int>("screen_interval");

  // Error checks
  if (interval < screen_interval)
    mooseError("\"screen_interval (" + Moose::stringify(screen_interval) +
               ")\" must be less than or equal to \"interval (" + Moose::stringify(interval) + ")\"");
  else if (interval > screen_interval)
  {
    if (interval % screen_interval)
      mooseError("\"screen_interval (" + Moose::stringify(screen_interval) +
                 ")\" must evenly divide \"interval (" + Moose::stringify(interval) + ")\"");
    else if (!output_initial)
      mooseError("\"interval (" + Moose::stringify(interval) + ") is set greater than \"screen_interval (" +
                 Moose::stringify(screen_interval) + ")\" and \"output_initial\" is set to false.");
  }

  output.interval(getParam<unsigned int>("interval"));
  output.screen_interval(getParam<unsigned int>("screen_interval"));
  output.iterationPlotStartTime(getParam<Real>("iteration_plot_start_time"));
  if(isParamValid("time_interval"))
   {
     Real time_interval_out = getParam<Real>("time_interval");
     output.setTimeIntervalOutput(time_interval_out);
     if(time_interval_out<=0)
     {
       mooseError("time interval must be positive");
     }
   }

  if(getParam<bool>("color_output"))
  {
    char * term_env = getenv("TERM");

    if(term_env)
    {
      std::string term(term_env);

      bool color = false;

      if(term == "xterm-256color")
        color = true;

      if(term == "xterm")
        color = true;

      if(color == true)
        _problem->setColorOutput(true);
    }
  }

 // Test to make sure that the user can write to the directory specified in file_base
  std::string base = "./" + getParam<OutFileBase>("file_base");
  base = base.substr(0, base.find_last_of('/'));

  // TODO: We have a function that tests read/write in the Parser namespace.  We should probably
  // use that instead of creating another one here
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + getParam<OutFileBase>("file_base"));

  _problem->getNonlinearSystem().printAllVariableNorms(getParam<bool>("all_var_norms"));
}
