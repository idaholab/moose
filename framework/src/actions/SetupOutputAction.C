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

#include "Moose.h"
#include "Parser.h"
#include "Output.h"
#include "FEProblem.h"
#include "Conversion.h"

#include "exodusII_io.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SetupOutputAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("file_base", "The desired solution output name without an extension (Defaults to the mesh file name + '_out' or 'out' if generating the mesh by some other means)");
  params.addParam<unsigned int>("interval", 1, "The interval at which timesteps are output to the solution file");
  params.addParam<unsigned int>("screen_interval", 1, "The interval at which postprocessors are output to the screen. This value must evenly divide \"interval\" so that postprocessors are calculated at corresponding solution timesteps. In addition, if \"screen_interval\" is strictly greater than \"interval\", \"output_initial\" must be set to true");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("nemesis", false, "Specifies that you would like Nemesis output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("tecplot_binary", false, "Specifies that you would like Tecplot binary output solution files(s)");
  params.addParam<bool>("xda", false, "Specifies that you would like xda output solution files(s)");
  params.addParam<bool>("postprocessor_screen", true, "Specifies that you would like PostProcessor output to the screen (stdout)");
  params.addParam<unsigned int>("max_pps_rows_screen", 0, "The maximum number of postprocessor values displayed on screen during a timestep");
  params.addParam<bool>("postprocessor_csv", false, "Specifies that you would like a PostProcessor comma seperated values file");
  params.addParam<bool>("postprocessor_ensight", false, "Specifies that you would like a PostProcessor ensight output file");
  params.addParam<bool>("postprocessor_gnuplot", false, "Specifies that you would like plots of the postprocessor output");
  params.addParam<std::string>("gnuplot_format", "ps", "Specifies which output format gnuplot will produce. Currently supported: ps, gif, and png");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  params.addParam<bool>("output_displaced", false, "Requests that displaced mesh files are written at each solve");
  params.addParam<bool>("output_solution_history", false, "Requests that the 'solution history' is output, the solution history is the number of nonlinear / linear solves that are done for each step.");

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<bool>("print_linear_residuals", false, "Specifies whether the linear residuals are printed during the solve");
#endif

  params.addParam<Real>("iteration_plot_start_time", std::numeric_limits<Real>::max(), "Specifies a time after which the solution will be written following each nonlinear iteration");

  params.addParam<bool>("perf_log",        false,    "Specifies whether or not the Performance log should be printed");
  params.addParam<bool>("show_setup_log_early", false, "Specifies whether or not the Setup Performance log should be printed before the first time step.  It will still be printed at the end if ""perf_log"" is also enabled and likewise disabled in ""perf_log"" is false");
  params.addParam<std::vector<std::string> >("output_variables", "A list of the variables that should be in the Exodus output file.  If this is not provided then all variables will be in the output.");
  params.addParam<bool>("elemental_as_nodal", false, "Output elemental variables also as nodal");
  params.addParam<bool>("exodus_inputfile_output", true, "Determines whether or not the input file is output to exodus - default (true)");

  return params;
}


SetupOutputAction::SetupOutputAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupOutputAction::setupOutputObject(Output &output, InputParameters & params)
{
  output.fileBase(params.get<std::string>("file_base"));

  mooseAssert(params.have_parameter<std::vector<std::string> >("output_variables"), "Output Variables are required");

  if (params.get<bool>("exodus"))
  {
    if (params.have_parameter<bool>("exodus_inputfile_output") && !params.get<bool>("exodus_inputfile_output"))
      output.add(Output::EXODUS, false);
    else
      output.add(Output::EXODUS, true);
  }
  if (params.get<bool>("nemesis")) output.add(Output::NEMESIS);
  if (params.get<bool>("gmv")) output.add(Output::GMV);
  if (params.get<bool>("tecplot")) output.add(Output::TECPLOT);
  if (params.get<bool>("tecplot_binary")) output.add(Output::TECPLOT_BIN);
  if (params.get<bool>("xda")) output.add(Output::XDA);
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

  // FIXME: _parser_handle._problem can be NULL !!!

  /// Determines whether we see the perf log early in a run or not
  _parser_handle._problem->setEarlyPerfLogPrint(getParam<bool>("show_setup_log_early"));

  FEProblem & problem = *_parser_handle._problem;
  Output & output = problem.out();                       // can't use use this with coupled problems on different meshes

  if (_parser_handle._problem != NULL)
  {
    FEProblem & fe_problem = *_parser_handle._problem;
    if(_pars.isParamValid("output_variables"))
    {
      output.setOutputVariables(getParam<std::vector<std::string> >("output_variables"));
    }
    else
    {
      if (getParam<bool>("elemental_as_nodal"))
      {
        // output all variables in the system
        output.setOutputVariables(fe_problem.getVariableNames());
      }
    }

    // If the user didn't provide a filename - see if the parser has a filename that we can use as a base
    if (!_pars.isParamValid("file_base"))
    {
      std::string input_file_name = _parser_handle.getFileName();
      mooseAssert(input_file_name != "", "Input Filename is NULL");
      size_t pos = input_file_name.find_last_of('.');
      mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
      _pars.set<std::string>("file_base") = input_file_name.substr(0,pos) + "_out";
    }
  }
  else
    _pars.set<std::string>("file_base") = "out";

  setupOutputObject(output, _pars);

  const bool output_initial = getParam<bool>("output_initial");
  problem.outputInitial(output_initial);

  if (_parser_handle._problem != NULL)
  {
    // TODO: handle this thru Problem interface
    FEProblem & fe_problem = *_parser_handle._problem;
    fe_problem._postprocessor_screen_output = getParam<bool>("postprocessor_screen");
    fe_problem._postprocessor_csv_output = getParam<bool>("postprocessor_csv");
    fe_problem._postprocessor_ensight_output = getParam<bool>("postprocessor_ensight");
    fe_problem._postprocessor_gnuplot_output = getParam<bool>("postprocessor_gnuplot");
    fe_problem._gnuplot_format = getParam<std::string>("gnuplot_format");
    fe_problem.setMaxPPSRowsScreen(getParam<unsigned int>("max_pps_rows_screen"));

    fe_problem.outputDisplaced(getParam<bool>("output_displaced"));
    fe_problem.outputSolutionHistory(getParam<bool>("output_solution_history"));

#ifdef LIBMESH_ENABLE_AMR
    Adaptivity & adapt = fe_problem.adaptivity();
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
  }

#ifdef LIBMESH_HAVE_PETSC
//  if (getParam<bool>("print_linear_residuals"))
//    PetscOptionsSetValue("-ksp_monitor", PETSC_NULL);
#endif

 // Test to make sure that the user can write to the directory specified in file_base
  std::string base = "./" + getParam<std::string>("file_base");
  base = base.substr(0, base.find_last_of('/'));

  // TODO: We have a function that tests read/write in the Parser namespace.  We should probably
  // use that instead of creating another one here
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + getParam<std::string>("file_base"));
}
