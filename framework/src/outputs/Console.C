//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Console.h"
#include "ConsoleUtils.h"
#include "FEProblem.h"
#include "EigenProblem.h"
#include "Postprocessor.h"
#include "PetscSupport.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "Moose.h"
#include "FormattedTable.h"
#include "NonlinearSystem.h"
#include "CommonOutputAction.h"

// libMesh includes
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", Console);

InputParameters
Console::validParams()
{
  // Enum for selecting the fit mode for the table when printed to the screen
  MooseEnum pps_fit_mode(FormattedTable::getWidthModes());

  // Get the parameters from the base class
  InputParameters params = TableOutput::validParams();
  params.addClassDescription("Object for screen output.");

  params += TableOutput::enableOutputTypes("system_information scalar postprocessor input");

  // Screen and file output toggles
  params.addParam<bool>("output_screen", true, "Output to the screen");
  params.addParam<bool>("output_file", false, "Output to the file");
  params.addParam<bool>(
      "show_multiapp_name", false, "Indent multiapp output using the multiapp name");

  // Table fitting options
  params.addParam<unsigned int>("max_rows",
                                15,
                                "The maximum number of postprocessor/scalar values "
                                "displayed on screen during a timestep (set to 0 "
                                "for unlimited)");
  params.addParam<MooseEnum>("fit_mode",
                             pps_fit_mode,
                             "Specifies the wrapping mode for post-processor tables that are "
                             "printed to the screen (ENVIRONMENT: Read \"MOOSE_PPS_WIDTH\" for "
                             "desired width (if not set, defaults to AUTO), AUTO: Attempt to "
                             "determine width automatically (serial only), <n>: Desired width");

  // Verbosity
  params.addParam<bool>("verbose", false, "Print detailed diagnostics on timestep calculation");

  // Basic table output controls
  params.addParam<bool>(
      "scientific_time", false, "Control the printing of time and dt in scientific notation");
  params.addParam<unsigned int>(
      "time_precision",
      "The number of significant digits that are printed on time related outputs");

  // Performance Logging
  params.addDeprecatedParam<bool>("perf_log",
                                  false,
                                  "If true, all performance logs will be printed. The "
                                  "individual log settings will override this option.",
                                  "Use PerfGraphOutput");
  params.addDeprecatedParam<unsigned int>(
      "perf_log_interval",
      0,
      "If set, the performance log will be printed every n time steps",
      "Use PerfGraphOutput instead");
  params.addParam<bool>("solve_log", "Toggles the printing of the 'Moose Test Performance' log");
  params.addDeprecatedParam<bool>(
      "perf_header",
      "Print the libMesh performance log header (requires that 'perf_log = true')",
      "Use PerfGraphOutput instead");

  params.addParam<bool>(
      "libmesh_log",
      true,
      "Print the libMesh performance log, requires libMesh to be configured with --enable-perflog");

  // Toggle printing of mesh information on adaptivity steps
  params.addParam<bool>("print_mesh_changed_info",
                        false,
                        "When true, each time the mesh is changed the mesh information is printed");

  // Toggle for printing variable norms
  params.addParam<bool>("outlier_variable_norms",
                        true,
                        "If true, outlier variable norms will be printed after each solve");
  params.addParam<bool>(
      "all_variable_norms", false, "If true, all variable norms will be printed after each solve");

  // Multipliers for coloring of variable residual norms
  std::vector<Real> multiplier;
  multiplier.push_back(0.8);
  multiplier.push_back(2);
  params.addParam<std::vector<Real>>("outlier_multiplier",
                                     multiplier,
                                     "Multiplier utilized to determine if a residual norm is an "
                                     "outlier. If the variable residual is less than "
                                     "multiplier[0] times the total residual it is colored red. "
                                     "If the variable residual is less than multiplier[1] times "
                                     "the average residual it is colored yellow.");

  // System information controls
  MultiMooseEnum info("framework mesh aux nonlinear relationship execution output",
                      "framework mesh aux nonlinear execution");
  params.addParam<MultiMooseEnum>("system_info",
                                  info,
                                  "List of information types to display "
                                  "('framework', 'mesh', 'aux', 'nonlinear', 'relationship', "
                                  "'execution', 'output')");

  // Advanced group
  params.addParamNamesToGroup("verbose show_multiapp_name system_info", "Advanced");

  // Performance log group
  params.addParamNamesToGroup("perf_log solve_log perf_header libmesh_log", "Perf Log");

  // Variable norms group
  params.addParamNamesToGroup("outlier_variable_norms all_variable_norms outlier_multiplier",
                              "Variable and Residual Norms");

  // Number formatting
  params.addParamNamesToGroup("scientific_time time_precision", "Time output formatting");

  // Table of postprocessor output formatting
  params.addParamNamesToGroup("max_rows fit_mode", "Table formatting");

  /*
   * The following modifies the default behavior from base class parameters. Notice the extra flag
   * on
   * the set method. This enables "quiet mode". This is done to allow for the proper detection
   * of user-modified parameters
   */
  // By default set System Information to output on initial
  params.set<ExecFlagEnum>("execute_system_information_on", /*quite_mode=*/true) = EXEC_INITIAL;

  // Change the default behavior of 'execute_on' to included nonlinear iterations and failed
  // timesteps
  params.set<ExecFlagEnum>("execute_on", /*quiet_mode=*/true) = {
      EXEC_INITIAL, EXEC_TIMESTEP_BEGIN, EXEC_LINEAR, EXEC_NONLINEAR, EXEC_FAILED};

  // By default postprocessors and scalar are only output at the end of a timestep
  params.set<ExecFlagEnum>("execute_postprocessors_on", /*quiet_mode=*/true) = {EXEC_INITIAL,
                                                                                EXEC_TIMESTEP_END};
  params.set<ExecFlagEnum>("execute_vector_postprocessors_on",
                           /*quiet_mode=*/true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.set<ExecFlagEnum>("execute_scalars_on", /*quiet_mode=*/true) = {EXEC_INITIAL,
                                                                         EXEC_TIMESTEP_END};
  params.set<ExecFlagEnum>("execute_reporters_on", /*quiet_mode=*/true) = {EXEC_INITIAL,
                                                                           EXEC_TIMESTEP_END};
  return params;
}

Console::Console(const InputParameters & parameters)
  : TableOutput(parameters),
    _max_rows(getParam<unsigned int>("max_rows")),
    _fit_mode(getParam<MooseEnum>("fit_mode")),
    _scientific_time(getParam<bool>("scientific_time")),
    _write_file(getParam<bool>("output_file")),
    _write_screen(getParam<bool>("output_screen")),
    _verbose(getParam<bool>("verbose")),
    _perf_log(getParam<bool>("perf_log")),
    _perf_log_interval(getParam<unsigned int>("perf_log_interval")),
    _solve_log(isParamValid("solve_log") ? getParam<bool>("solve_log") : _perf_log),
    _libmesh_log(getParam<bool>("libmesh_log")),
    _perf_header(isParamValid("perf_header") ? getParam<bool>("perf_header") : _perf_log),
    _all_variable_norms(getParam<bool>("all_variable_norms")),
    _outlier_variable_norms(getParam<bool>("outlier_variable_norms")),
    _outlier_multiplier(getParam<std::vector<Real>>("outlier_multiplier")),
    _precision(isParamValid("time_precision") ? getParam<unsigned int>("time_precision") : 0),
    _console_buffer(_app.getOutputWarehouse().consoleBuffer()),
    _old_linear_norm(std::numeric_limits<Real>::max()),
    _old_nonlinear_norm(std::numeric_limits<Real>::max()),
    _print_mesh_changed_info(getParam<bool>("print_mesh_changed_info")),
    _system_info_flags(getParam<MultiMooseEnum>("system_info")),
    _allow_changing_sysinfo_flag(true),
    _last_message_ended_in_newline(true)
{
  // Apply the special common console flags (print_...)
  ActionWarehouse & awh = _app.actionWarehouse();
  const auto actions = awh.getActions<CommonOutputAction>();
  mooseAssert(actions.size() <= 1, "Should not be more than one CommonOutputAction");
  const Action * common = actions.empty() ? nullptr : *actions.begin();

  if (!parameters.isParamSetByUser("execute_on"))
  {
    // Honor the 'print_linear_residuals' option, only if 'linear' has not been set in 'execute_on'
    // by the user
    if (common && common->getParam<bool>("print_linear_residuals"))
      _execute_on.push_back("linear");
    else
      _execute_on.erase("linear");
    if (common && common->getParam<bool>("print_nonlinear_residuals"))
      _execute_on.push_back("nonlinear");
    else
      _execute_on.erase("nonlinear");
  }

  if (!_pars.isParamSetByUser("perf_log") && common && common->getParam<bool>("print_perf_log"))
  {
    _perf_log = true;
    _solve_log = true;
  }

  // Append the common 'execute_on' to the setting for this object
  // This is unique to the Console object, all other objects inherit from the common options
  if (common)
  {
    const ExecFlagEnum & common_execute_on = common->getParam<ExecFlagEnum>("execute_on");
    for (auto & mme : common_execute_on)
      _execute_on.push_back(mme);
  }

  // If --show-outputs is used, enable it
  if (_app.getParam<bool>("show_outputs"))
    _system_info_flags.push_back("output");
}

Console::~Console()
{
  // Write the libMesh log
  if (_libmesh_log)
    write(libMesh::perflog.get_perf_info(), false);

  // Write the file output stream
  writeStreamToFile();

  // Disable logging so that the destructor in libMesh doesn't print
  Moose::perf_log.disable_logging();
  libMesh::perflog.disable_logging();
}

void
Console::initialSetup()
{
  // Only allow the main app to change the perf_log settings.
  if (_app.name() == "main")
  {
    // Disable libMesh log
    if (!_libmesh_log)
      libMesh::perflog.disable_logging();
  }

  // system info flag can be changed only before console initial setup
  _allow_changing_sysinfo_flag = false;

  // If execute_on = 'initial' perform the output
  if (wantOutput("system_information", EXEC_INITIAL))
    outputSystemInformation();

  // Call the base class method
  TableOutput::initialSetup();

  // If file output is desired, wipe out the existing file if not recovering
  if (!_app.isRecovering())
    writeStreamToFile(false);

  // Enable verbose output if Executioner has it enabled
  if (_app.getExecutioner()->isParamValid("verbose") &&
      _app.getExecutioner()->getParam<bool>("verbose"))
    _verbose = true;

  // If the user adds "final" to the execute on, append this to the postprocessors, scalars, etc.,
  // but only
  // if the parameter (e.g., postprocessor_execute_on) has not been modified by the user.
  if (_execute_on.contains("final"))
  {
    if (!_pars.isParamSetByUser("postprocessor_execute_on"))
      _advanced_execute_on["postprocessors"].push_back("final");
    if (!_pars.isParamSetByUser("scalars_execute_on"))
      _advanced_execute_on["scalars"].push_back("final");
    if (!_pars.isParamSetByUser("vector_postprocessor_execute_on"))
      _advanced_execute_on["vector_postprocessors"].push_back("final");
  }
}

std::string
Console::filename()
{
  return _file_base + ".txt";
}

void
Console::timestepSetup()
{
  writeTimestepInformation(/*output_dt = */ true);
}

void
Console::output(const ExecFlagType & type)
{
  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // Output the system information first; this forces this to be the first item to write by default
  // However, 'output_system_information_on' still operates correctly, so it may be changed by the
  // user
  if (wantOutput("system_information", type) && !(type == EXEC_INITIAL))
    outputSystemInformation();

  // Write the input
  if (wantOutput("input", type))
    outputInput();

  // Write the timestep information ("Time Step 0 ..."), this is controlled with "execute_on"
  // We only write the initial and final here. All of the intermediate outputs will be written
  // through timestepSetup.
  if (type == EXEC_INITIAL && _execute_on.contains(EXEC_INITIAL))
    writeTimestepInformation(/*output_dt = */ false);
  else if (type == EXEC_FINAL && _execute_on.contains(EXEC_FINAL))
  {
    if (wantOutput("postprocessors", type) || wantOutput("scalars", type))
      _console << "\nFINAL:\n";
  }

  // Print Non-linear Residual (control with "execute_on")
  if (type == EXEC_NONLINEAR && _execute_on.contains(EXEC_NONLINEAR))
  {
    if (_nonlinear_iter == 0)
      _old_nonlinear_norm = std::numeric_limits<Real>::max();

    _console << std::right << std::setw(2) << _nonlinear_iter
             << " Nonlinear |R| = " << outputNorm(_old_nonlinear_norm, _norm) << '\n';

    _old_nonlinear_norm = _norm;
  }

  // Print Linear Residual (control with "execute_on")
  else if (type == EXEC_LINEAR && _execute_on.contains(EXEC_LINEAR))
  {
    if (_linear_iter == 0)
      _old_linear_norm = std::numeric_limits<Real>::max();

    _console << std::right << std::setw(7) << _linear_iter
             << " Linear |R| = " << outputNorm(_old_linear_norm, _norm) << '\n';

    _old_linear_norm = _norm;
  }

  // Write variable norms
  else if (type == EXEC_TIMESTEP_END)
  {
    if (_perf_log_interval && _t_step % _perf_log_interval == 0)
      write(Moose::perf_log.get_perf_info(), false);
    writeVariableNorms();
  }

  if (wantOutput("postprocessors", type))
    outputPostprocessors();

  if (wantOutput("scalars", type))
    outputScalarVariables();

  if (wantOutput("reporters", type))
    outputReporters();

  // Write the file
  writeStreamToFile();

  _console << std::flush;
}

void
Console::writeStreamToFile(bool append)
{
  if (!_write_file)
    return;

  // Create the stream
  std::ofstream output;

  // Open the file
  if (append)
    output.open(filename().c_str(), std::ios::app | std::ios::out);
  else
    output.open(filename().c_str(), std::ios::trunc);

  if (output.fail())
    mooseError("Unable to open file ", filename());

  std::string s = _file_output_stream.str();
  // Write contents of file output stream and close the file
  output << MooseUtils::removeColor(s);
  output.close();

  // Clear the file output stream
  _file_output_stream.str("");
}

void
Console::writeTimestepInformation(bool output_dt)
{
  // Stream to build the time step information
  std::stringstream oss;

  // Write timestep data for transient executioners
  if (_transient)
  {
    // Write time step and time information
    oss << "\nTime Step " << timeStep();

    // Set precision
    if (_precision > 0)
      oss << std::setw(_precision) << std::setprecision(_precision) << std::setfill('0')
          << std::showpoint;

    // Show scientific notation
    if (_scientific_time)
      oss << std::scientific;

    // Print the time
    oss << ", time = " << time();

    if (output_dt)
    {
      if (!_verbose)
        // Show the time delta information
        oss << ", dt = " << std::left << dt();

      // Show old time information, if desired on separate lines
      else
      {
        oss << '\n'
            << std::right << std::setw(21) << std::setfill(' ') << "old time = " << std::left
            << timeOld() << '\n';

        // Show the time delta information
        oss << std::right << std::setw(21) << std::setfill(' ') << "dt = " << std::left << dt()
            << '\n';

        // Show the old time delta information, if desired
        if (_verbose)
          oss << std::right << std::setw(21) << std::setfill(' ') << "old dt = " << _dt_old << '\n';
      }
    }

    oss << '\n';

    // Output to the screen
    _console << oss.str() << std::flush;
  }
}

void
Console::writeVariableNorms()
{
  // If all_variable_norms is true, then so should outlier printing
  if (_all_variable_norms)
    _outlier_variable_norms = true;

  // if we are not priting anything, let's not waste time computing the norms below and just exit
  // this call
  if ((_all_variable_norms == false) && (_outlier_variable_norms == false))
    return;

  // if it is an eigenvalue prolblem, we do not know to define RHS,
  // and then we do not know how to compute variable norms
  if (dynamic_cast<EigenProblem *>(_problem_ptr) != nullptr)
    return;

  // Flag set when header prints
  bool header = false;

  // String stream for variable norm information
  std::ostringstream oss;

  // Get a references to the NonlinearSystem and libMesh system
  NonlinearSystemBase & nl = _problem_ptr->getNonlinearSystemBase();
  System & sys = nl.system();

  // Storage for norm outputs
  std::map<std::string, Real> other;
  std::map<std::string, Real> outlier;

  // Average norm
  unsigned int n_vars = sys.n_vars();
  Real avg_norm = (nl.nonlinearNorm() * nl.nonlinearNorm()) / n_vars;

  // Compute the norms for each of the variables
  for (unsigned int i = 0; i < n_vars; i++)
  {
    // Compute the norm and extract the variable name
    Real var_norm = sys.calculate_norm(nl.RHS(), i, DISCRETE_L2);
    var_norm *= var_norm; // use the norm squared
    std::string var_name = sys.variable_name(i);

    // Outlier if the variable norm is greater than twice (default) of the average norm
    if (_outlier_variable_norms && (var_norm > _outlier_multiplier[1] * avg_norm))
    {
      // Print the header
      if (!header)
      {
        oss << "\nOutlier Variable Residual Norms:\n";
        header = true;
      }

      // Set the color, RED if the variable norm is 0.8 (default) of the total norm
      std::string color = COLOR_YELLOW;
      if (_outlier_variable_norms && (var_norm > _outlier_multiplier[0] * avg_norm * n_vars))
        color = COLOR_RED;

      // Display the residual
      oss << "  " << var_name << ": " << std::scientific << color << std::sqrt(var_norm)
          << COLOR_DEFAULT << '\n';
    }

    // GREEN
    else if (_all_variable_norms)
    {
      // Print the header if it doesn't already exist
      if (!header)
      {
        oss << "\nVariable Residual Norms:\n";
        header = true;
      }
      oss << "  " << var_name << ": " << std::scientific << COLOR_GREEN << std::sqrt(var_norm)
          << COLOR_DEFAULT << '\n';
    }
  }

  // Update the output streams
  _console << oss.str() << std::flush;
}

// Quick helper to output the norm in color
std::string
Console::outputNorm(const Real & old_norm, const Real & norm, const unsigned int precision)
{
  std::string color = COLOR_GREEN;

  // Red if the residual went up... or if the norm is nan
  if (norm != norm || norm > old_norm)
    color = COLOR_RED;
  // Yellow if change is less than 5%
  else if ((old_norm - norm) / old_norm <= 0.05)
    color = COLOR_YELLOW;

  std::stringstream oss;
  oss << std::scientific << std::setprecision(precision) << color << norm << COLOR_DEFAULT;

  return oss.str();
}

void
Console::outputInput()
{
  if (!_write_screen && !_write_file)
    return;

  std::ostringstream oss;
  oss << "--- " << _app.getInputFileName()
      << " ------------------------------------------------------";
  _app.actionWarehouse().printInputFile(oss);
  _console << oss.str() << std::endl;
}

void
Console::outputPostprocessors()
{
  TableOutput::outputPostprocessors();

  if (!_postprocessor_table.empty())
  {
    std::stringstream oss;
    oss << "\nPostprocessor Values:\n";
    _postprocessor_table.sortColumns();
    _postprocessor_table.printTable(oss, _max_rows, _fit_mode);
    _console << oss.str() << std::endl;
  }
}

void
Console::outputReporters()
{
  TableOutput::outputReporters();

  if (!_reporter_table.empty())
  {
    std::stringstream oss;
    oss << "\nReporter Values:\n";
    _reporter_table.sortColumns();
    _reporter_table.printTable(oss, _max_rows, _fit_mode);
    _console << oss.str() << '\n';
  }
}

void
Console::outputScalarVariables()
{
  TableOutput::outputScalarVariables();

  if (!_scalar_table.empty())
  {
    std::stringstream oss;
    oss << "\nScalar Variable Values:\n";
    if (processor_id() == 0)
    {
      _scalar_table.sortColumns();
      _scalar_table.printTable(oss, _max_rows, _fit_mode);
    }
    _console << oss.str() << std::endl;
  }
}

void
Console::outputSystemInformation()
{
  // skip system information output for sub-apps other than the zero-th of a MultiApp
  // because they are using the same inputs and are most likely having the same information.
  if (_app.multiAppNumber() > 0)
    return;

  if (_system_info_flags.contains("framework"))
    _console << ConsoleUtils::outputFrameworkInformation(_app);

  if (_system_info_flags.contains("mesh"))
    _console << ConsoleUtils::outputMeshInformation(*_problem_ptr);

  if (_system_info_flags.contains("nonlinear"))
  {
    std::string output = ConsoleUtils::outputNonlinearSystemInformation(*_problem_ptr);
    if (!output.empty())
      _console << "Nonlinear System:\n" << output;
  }

  if (_system_info_flags.contains("aux"))
  {
    std::string output = ConsoleUtils::outputAuxiliarySystemInformation(*_problem_ptr);
    if (!output.empty())
      _console << "Auxiliary System:\n" << output;
  }

  if (_system_info_flags.contains("relationship"))
  {
    std::string output = ConsoleUtils::outputRelationshipManagerInformation(_app);
    if (!output.empty())
      _console << "Relationship Managers:\n" << output;
  }

  if (_system_info_flags.contains("execution"))
    _console << ConsoleUtils::outputExecutionInformation(_app, *_problem_ptr);

  if (_system_info_flags.contains("output"))
    _console << ConsoleUtils::outputOutputInformation(_app);

  // Output the legacy flags, these cannot be turned off so they become annoying to people.
  _console << ConsoleUtils::outputLegacyInformation(_app);

  _console << std::flush;
}

void
Console::meshChanged()
{
  if (_print_mesh_changed_info)
  {
    _console << ConsoleUtils::outputMeshInformation(*_problem_ptr, /*verbose = */ false);

    std::string output = ConsoleUtils::outputNonlinearSystemInformation(*_problem_ptr);
    if (!output.empty())
      _console << "Nonlinear System:\n" << output;

    output = ConsoleUtils::outputAuxiliarySystemInformation(*_problem_ptr);
    if (!output.empty())
      _console << "Auxiliary System:\n" << output;

    _console << std::flush;
  }
}

void
Console::write(std::string message, bool indent /*=true*/)
{
  // Do nothing if the message is empty, writing empty strings messes with multiapp indenting
  if (message.empty())
    return;

  // Write the message to file
  if (_write_file)
    _file_output_stream << message;

  // The empty case gets the right behavior, even though the boolean is technically wrong
  bool this_message_ends_in_newline = message.empty() ? true : (message.back() == '\n');
  bool this_message_starts_with_newline = message.empty() ? true : (message.front() == '\n');

  // Apply MultiApp indenting
  if ((this_message_starts_with_newline || _last_message_ended_in_newline) && indent &&
      _app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), message);

  // Write message to the screen
  if (_write_screen)
    Moose::out << message;

  _last_message_ended_in_newline = this_message_ends_in_newline;
}

void
Console::mooseConsole(const std::string & message)
{
  // Write the messages
  write(message);

  // Flush the stream to the screen
  Moose::out << std::flush;
}

void
Console::petscSetupOutput()
{
  char c[] = {
      32,  47, 94,  92,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  47, 94,  92,  13,  10,  124, 32,  32,  32,  92,  95,  47,
      94,  92, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  47,  94,  92,  95,  47,
      32,  32, 32,  124, 13,  10,  124, 32, 32,  32,  32,  32,  32,  32,  32,  92,  95,  47,  94,
      92,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  47,  94,  92,  95,  47, 32,  32,  32,  32,  32,  32,  32,  32,  124, 13,  10,
      32,  92, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  92,  95,  47,  94,  92,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  47,  94,  92,  95,  47,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  47,  13,  10,  32,  32,  92,  95,  95,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  92,  95,  95,  95,  45,  45,  45,
      95,  95, 95,  47,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  95,
      95,  47, 13,  10,  32,  32,  32,  32, 32,  45,  45,  45,  95,  95,  95,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  47,  32,  32, 32,  32,  32,  32,  32,  92,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  95,  95,  95,  45, 45,  45,  13,  10,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  45,  45,  45,  95, 95,  95,  32,  32,  124, 32,  32,  32,  32,  32,  32,
      32,  32, 32,  124, 32,  32,  95,  95, 95,  45,  45,  45,  13,  10,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  45,  45,  124, 32,  32,  95,  32,
      32,  32, 95,  32,  32,  124, 45,  45, 13,  10,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  124, 32,  32,  124, 111, 124, 32,  124, 111, 124,
      32,  32, 124, 13,  10,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  47,  32,  32,  32,  32, 45,  32,  32,  32,  45,  32,  32,  32,  32,  92,  13,
      10,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  124, 32,
      32,  32, 32,  32,  32,  95,  95,  95, 32,  32,  32,  32,  32,  32,  124, 13,  10,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  47,  32,  32,  32,  32,  32,
      45,  45, 32,  32,  32,  45,  45,  32, 32,  32,  32,  32,  92,  13,  10,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  47,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  92,  13,  10,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 124, 32,  32,  32,  32,  32,  32,  32,  47,  92,  32,
      32,  32, 32,  32,  47,  92,  32,  32, 32,  32,  32,  32,  32,  124, 13,  10,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  92,  32,  32,  92,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  47,  32,  32,  47,  13,  10,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  47,  92,  32,  32,  92,  95,  95,  95,  95,
      95,  95, 95,  95,  95,  95,  95,  95, 32,  47,  32,  32,  47,  92,  13,  10,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  47,  32,  32,  92,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  47,  32,  32,  92,  13,  10,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  47,  32,  32,  32,  32,  92,  32,  32,  32,  32,
      32,  39, 95,  95,  95,  39,  32,  32, 32,  32,  32,  47,  32,  32,  32,  32,  92,  13,  10,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  47,  92,  32,  32,  32,  32,  32,  92,  32,
      45,  45, 95,  95,  45,  45,  45,  95, 95,  45,  45,  32,  47,  32,  32,  32,  32,  32,  47,
      92,  13, 10,  32,  32,  32,  32,  32, 32,  32,  32,  32,  47,  32,  32,  92,  47,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      92,  47, 32,  32,  92,  13,  10,  32, 32,  32,  32,  32,  32,  32,  32,  47,  32,  32,  32,
      47,  32, 32,  32,  32,  32,  32,  32, 77,  46,  79,  46,  79,  46,  83,  46,  69,  32,  32,
      32,  32, 32,  32,  32,  92,  32,  32, 32,  92,  13,  10,  32,  32,  32,  32,  32,  32,  32,
      47,  32, 32,  32,  124, 32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  124, 32,  32,  32,  92,  13,  10,  32,
      32,  32, 32,  32,  32,  124, 32,  32, 32,  32,  124, 45,  45,  45,  45,  45,  45,  45,  45,
      45,  45, 45,  45,  45,  45,  45,  45, 45,  45,  45,  45,  45,  45,  45,  45,  45,  124, 32,
      32,  32, 32,  124, 13,  10,  32,  32, 32,  32,  32,  32,  32,  92,  32,  32,  32,  32,  92,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  47,  32,  32,  32, 32,  47,  13,  10,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 92,  92,  32,  92,  95,  92, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 47,  95,  47,  32,  47,  47,  13,  10,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 45,  45,  32,  32,  92,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  47,  32,  32,  45,  45,  13,  10,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  32,  32,  32,  124, 32,  32,  45,
      45,  45, 95,  95,  95,  95,  95,  45, 45,  45,  32,  32,  124, 13,  10,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  32,  32,  32,  124, 32,  32,  32,  32,  32,  124,
      32,  32, 32,  124, 32,  32,  32,  32, 32,  124, 13,  10,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  32,  32,  32, 32,  124, 32,  32,  32,  32,  32,  124, 32,  32,  32,
      124, 32, 32,  32,  32,  32,  124, 13, 10,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  32,  32,  47,  32,  86, 32,  32,  32,  32,  32,  92,  32,  47,  32,  32,  32,
      32,  86, 32,  32,  92,  13,  10,  32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32, 32,  124, 95,  124, 95,  95, 95,  95,  95,  124, 32,  124, 95,  95,  95,  95,  124,
      95,  95, 124};
  Moose::out << std::string(c) << std::endl << std::endl;
}
