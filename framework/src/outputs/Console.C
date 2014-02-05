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
#include "Console.h"
#include "FEProblem.h"
#include "Postprocessor.h"
#include "PetscSupport.h"
#include "Executioner.h"
#include "MooseApp.h"

template<>
InputParameters validParams<Console>()
{
  // Enum for selecting the fit mode for the table when printed to the screen
  MooseEnum pps_fit_mode(FormattedTable::getWidthModes());

  // Get the parameters from the base class
  InputParameters params = validParams<TableOutputBase>();

  // Table fitting options
  params.addParam<unsigned int>("max_rows", 15, "The maximum number of postprocessor/scalar values displayed on screen during a timestep (set to 0 for unlimited)");
  params.addParam<MooseEnum>("fit_mode", pps_fit_mode, "Specifies the wrapping mode for post-processor tables that are printed to the screen (ENVIRONMENT: Read \"PPS_WIDTH\" for desired width, AUTO: Attempt to determine width automatically (serial only), <n>: Desired width");

  // Basic table output controls
  params.addParam<bool>("color_output", true, "If true, color will be added to the output");
  params.addParam<bool>("linear_residuals", true, "Specifies whether the linear residuals are printed during the solve");
  params.addParam<bool>("nonlinear_residuals", true, "Specifies whether the nonlinear residuals are printed during the solve");
  params.addParam<bool>("system_information", true, "Toggles the display of the system information prior to the solve");

  // Performance Logging
  params.addParam<bool>("perf_log", false, "If true, the performance log will be printed");
  params.addParam<bool>("show_setup_log_early", false, "Specifies whether or not the Setup Performance log should be printed before the first time step.  It will still be printed at the end if ""perf_log"" is also enabled and likewise disabled if ""perf_log"" is false");

  // Advanced group
  params.addParamNamesToGroup("max_rows fit_node", "Advanced");

  // Performance log group
  params.addParamNamesToGroup("perf_log show_setup_log_early", "Performance Log");

  return params;
}

Console::Console(const std::string & name, InputParameters parameters) :
    TableOutputBase(name, parameters),
    _max_rows(getParam<unsigned int>("max_rows")),
    _fit_mode(getParam<MooseEnum>("fit_mode")),
    _use_color(false),
    _print_linear(getParam<bool>("linear_residuals")),
    _print_nonlinear(getParam<bool>("nonlinear_residuals")),
    _system_information(getParam<bool>("system_information"))
{

  // Disable Perf Log if requested
  if (!getParam<bool>("perf_log"))
  {
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
  }

  // Set the printing of the performane log
  if (getParam<bool>("show_setup_log_early"))
    Moose::setup_perf_log.print_log();

  // Set output coloring
  if(getParam<bool>("color_output"))
  {
    char * term_env = getenv("TERM");

    if(term_env)
    {
      std::string term(term_env);
      if(term == "xterm-256color" || term == "xterm")
        _use_color = true;
    }
  }

  // Call the correct PETSc output setup functions
  petscOutput();
}

Console::~Console()
{
}

void
Console::output()
{
  // Call the correct PETSc Output functions
  petscOutput();

  // Call the base class output function
  OutputBase::output();
}

void
Console::petscOutput()
{
  // Enable/disable the printing of linear residuals
  if (_print_nonlinear)
  {
    Moose::PetscSupport::petscPrintNonlinearResiduals(_problem_ptr, _use_color);

    // Enable/disable the printing of nonlinear residuals
    if (_print_linear)
      Moose::PetscSupport::petscPrintNonlinearResiduals(_problem_ptr, _use_color);
  }
}

void
Console::outputPostprocessors()
{
  TableOutputBase::outputPostprocessors();

  if (!_postprocessor_table.empty())
  {
    Moose::out << "\nPostprocessor Values:\n";
    _postprocessor_table.printTable(Moose::out, _max_rows, _fit_mode);
    Moose::out << std::endl;
  }
}

void
Console::outputScalarVariables()
{
  TableOutputBase::outputScalarVariables();

  if (!_scalar_table.empty())
  {
    Moose::out << "\nScalar AuxVariable Values:\n";
    _scalar_table.printTable(Moose::out, _max_rows, _fit_mode);
    Moose::out << std::endl;
  }
}

void
Console::petscSetupOutput()
{
char c[] =  {32,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,13,10,124,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,124,13,10,124,32,32,32,32,32,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,32,32,32,32,32,124,13,10,32,92,32,32,32,32,32,32,32,32,32,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,32,32,32,32,32,32,32,32,32,47,13,10,32,32,92,95,95,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,95,95,95,45,45,45,95,95,95,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,95,95,47,13,10,32,32,32,32,32,45,45,45,95,95,95,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,32,92,32,32,32,32,32,32,32,32,32,95,95,95,45,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,45,45,45,95,95,95,32,32,124,32,32,32,32,32,32,32,32,32,124,32,32,95,95,95,45,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,45,45,124,32,32,95,32,32,32,95,32,32,124,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,124,111,124,32,124,111,124,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,45,32,32,32,45,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,32,95,95,95,32,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,45,45,32,32,32,45,45,32,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,32,32,47,92,32,32,32,32,32,47,92,32,32,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,92,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,47,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,47,92,32,32,92,95,95,95,95,95,95,95,95,95,95,95,95,32,47,32,32,47,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,92,32,32,32,32,32,39,95,95,95,39,32,32,32,32,32,47,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,47,92,32,32,32,32,32,92,32,45,45,95,95,45,45,45,95,95,45,45,32,47,32,32,32,32,32,47,92,13,10,32,32,32,32,32,32,32,32,32,47,32,32,92,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,47,32,32,92,13,10,32,32,32,32,32,32,32,32,47,32,32,32,47,32,32,32,32,32,32,32,77,46,79,46,79,46,83,46,69,32,32,32,32,32,32,32,92,32,32,32,92,13,10,32,32,32,32,32,32,32,47,32,32,32,124,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,92,13,10,32,32,32,32,32,32,124,32,32,32,32,124,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,124,32,32,32,32,124,13,10,32,32,32,32,32,32,32,92,32,32,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,47,13,10,32,32,32,32,32,32,32,32,32,92,92,32,92,95,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,95,47,32,47,47,13,10,32,32,32,32,32,32,32,32,32,32,32,45,45,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,45,45,45,95,95,95,95,95,45,45,45,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,124,32,32,32,124,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,124,32,32,32,124,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,86,32,32,32,32,32,92,32,47,32,32,32,32,86,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,95,124,95,95,95,95,95,124,32,124,95,95,95,95,124,95,95,124};
Moose::out << std::string(c) << std::endl << std::endl;
}
