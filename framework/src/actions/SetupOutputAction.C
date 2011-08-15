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
#include "Executioner.h"
#include "Output.h"
#include "MProblem.h"

#include "exodusII_io.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SetupOutputAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("file_base", "out", "The desired solution output name without an extension");
  params.addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("nemesis", false, "Specifies that you would like Nemesis output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("tecplot_binary", false, "Specifies that you would like Tecplot binary output solution files(s)");
  params.addParam<bool>("xda", false, "Specifies that you would like xda output solution files(s)");
  params.addParam<bool>("postprocessor_screen", true, "Specifies that you would like PostProcessor output to the screen (stdout)");
  params.addParam<bool>("postprocessor_csv", false, "Specifies that you would like a PostProcessor comma seperated values file");
  params.addParam<bool>("postprocessor_ensight", false, "Specifies that you would like a PostProcessor ensight output file");
  params.addParam<bool>("postprocessor_gnuplot", false, "Specifies that you would like plots of the postprocessor output");
  params.addParam<std::string>("gnuplot_format", "ps", "Specifies which output format gnuplot will produce. Currently supported: ps, gif, and png");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  params.addParam<bool>("output_displaced", false, "Requests that displaced mesh files are written at each solve");

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<bool>("print_linear_residuals", false, "Specifies whether the linear residuals are printed during the solve");
#endif

  params.addParam<Real>("iteration_plot_start_time", std::numeric_limits<Real>::max(), "Specifies a time after which the solution will be written following each nonlinear iteration");

  params.addParam<bool>("perf_log",        false,    "Specifies whether or not the Performance log should be printed");
  params.addParam<bool>("show_setup_log_early", false, "Specifies whether or not the Setup Performance log should be printed before the first time step.  It will still be printed at the end if ""perf_log"" is also enabled and likewise disabled in ""perf_log"" is false");
  params.addParam<std::vector<std::string> >("output_variables", "A list of the variables that should be in the Exodus output file.  If this is not provided then all variables will be in the output.");

  return params;
}


SetupOutputAction::SetupOutputAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupOutputAction::setupOutputObject(Output &output, InputParameters & params)
{
  output.fileBase(getParam<std::string>("file_base"));

  mooseAssert(params.have_parameter<std::vector<std::string> >("output_variables"), "Output Variables are required");

  output.setOutputVariables(params.get<std::vector<std::string> >("output_variables"));

  if (params.get<bool>("exodus")) output.add(Output::EXODUS);
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
 
  /// Determines whether we see the perf log early in a run or not
  _parser_handle._problem->setEarlyPerfLogPrint(getParam<bool>("show_setup_log_early"));

  Executioner * exec = _parser_handle._executioner;
  Problem & problem = exec->problem();
  Output & output = problem.out();                       // can't use use this with coupled problems on different meshes

  if(!_pars.isParamValid("output_variables") && _parser_handle._problem != NULL)
  {
    MProblem & mproblem = *_parser_handle._problem;
    _pars.set<std::vector<std::string> >("output_variables") = mproblem.getVariableNames();
  }

  setupOutputObject(output, _pars);

  // TODO: Really? We need to set this variable on two objects???
  exec->outputInitial(getParam<bool>("output_initial"));
  problem.outputInitial(getParam<bool>("output_initial"));

  if (_parser_handle._problem != NULL)
  {
    // TODO: handle this thru Problem interface
    MProblem & mproblem = *_parser_handle._problem;
    mproblem._postprocessor_screen_output = getParam<bool>("postprocessor_screen");
    mproblem._postprocessor_csv_output = getParam<bool>("postprocessor_csv");
    mproblem._postprocessor_ensight_output = getParam<bool>("postprocessor_ensight");
    mproblem._postprocessor_gnuplot_output = getParam<bool>("postprocessor_gnuplot");
    mproblem._gnuplot_format = getParam<std::string>("gnuplot_format");

    mproblem.outputDisplaced(getParam<bool>("output_displaced"));

#ifdef LIBMESH_ENABLE_AMR
    Adaptivity & adapt = mproblem.adaptivity();
    if (adapt.isOn())
      output.sequence(true);
#endif //LIBMESH_ENABLE_AMR

    output.interval(getParam<int>("interval"));
    output.iterationPlotStartTime(getParam<Real>("iteration_plot_start_time"));
  }

#ifdef LIBMESH_HAVE_PETSC
//  if (getParam<bool>("print_linear_residuals"))
//    PetscOptionsSetValue("-ksp_monitor", PETSC_NULL);
#endif

 // Test to make sure that the user can write to the directory specified in file_base
  std::string base = "./" + getParam<std::string>("file_base");
  base = base.substr(0, base.find_last_of('/'));
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + getParam<std::string>("file_base"));

}
