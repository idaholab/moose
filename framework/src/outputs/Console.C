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
#include "pcrecpp.h"
#include "Moose.h"

template<>
InputParameters validParams<Console>()
{
  // Enum for selecting the fit mode for the table when printed to the screen
  MooseEnum pps_fit_mode(FormattedTable::getWidthModes());

  // Get the parameters from the base class
  InputParameters params = validParams<TableOutput>();
  params += TableOutput::enableOutputTypes("system_information scalar postprocessor input");

  // Screen and file output toggles
  params.addParam<bool>("output_screen", true, "Output to the screen");
  params.addParam<bool>("output_file", false, "Output to the file");
  params.addParam<bool>("show_multiapp_name", false, "Indent multiapp output using the multiapp name");

  // Table fitting options
  params.addParam<unsigned int>("max_rows", 15, "The maximum number of postprocessor/scalar values displayed on screen during a timestep (set to 0 for unlimited)");
  params.addParam<MooseEnum>("fit_mode", pps_fit_mode, "Specifies the wrapping mode for post-processor tables that are printed to the screen (ENVIRONMENT: Read \"MOOSE_PPS_WIDTH\" for desired width, AUTO: Attempt to determine width automatically (serial only), <n>: Desired width");

  // Verbosity
  params.addParam<bool>("verbose", false, "Print detailed diagnostics on timestep calculation");
  params.addParam<bool>("show_output_on", false, "Print the output execution with the system information");

  // Basic table output controls
  params.addParam<bool>("scientific_time", false, "Control the printing of time and dt in scientific notation");
  params.addParam<unsigned int>("time_precision", "The number of significant digits that are printed on time related outputs");

  // Performance Logging
  params.addParam<bool>("perf_log", false, "If true, all performance logs will be printed. The individual log settings will override this option.");
  params.addParam<bool>("setup_log_early", false, "Specifies whether or not the Setup Performance log should be printed before the first time step.  It will still be printed at the end if ""perf_log"" is also enabled and likewise disabled if ""perf_log"" is false");
  params.addParam<bool>("setup_log", "Toggles the printing of the 'Setup Performance' log");
  params.addParam<bool>("solve_log", "Toggles the printing of the 'Moose Test Performance' log");
  params.addParam<bool>("perf_header", "Print the libMesh performance log header (requires that 'perf_log = true')");

#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
  params.addParam<bool>("libmesh_log", true, "Print the libMesh performance log, requires libMesh to be configured with --enable-perflog");
#endif

  // Toggle for printing variable normals
  params.addParam<bool>("outlier_variable_norms", true, "If true, outlier variable norms will be printed after each solve");
  params.addParam<bool>("all_variable_norms", false, "If true, all variable norms will be printed after each solve");

  // Multipliers for coloring of variable residual norms
  std::vector<Real> multiplier;
  multiplier.push_back(0.8);
  multiplier.push_back(2);
  params.addParam<std::vector<Real> >("outlier_multiplier", multiplier, "Multiplier utilized to determine if a residual norm is an outlier. If the variable residual is less than multiplier[0] times the total residual it is colored red. If the variable residual is less than multiplier[1] times the average residual it is colored yellow.");


  // Advanced group
  params.addParamNamesToGroup("max_rows fit_node verbose show_multiapp_name", "Advanced");

  // Performance log group
  params.addParamNamesToGroup("perf_log setup_log_early setup_log solve_log perf_header", "Perf Log");
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
  params.addParamNamesToGroup("libmesh_log", "Performance Log");
#endif

  // Variable norms group
  params.addParamNamesToGroup("outlier_variable_norms all_variable_norms outlier_multiplier", "Norms");

  /*
   * The following modifies the default behavior from base class parameters. Notice the extra flag on
   * the set method. This enables "quiet mode". This is done to allow for the proper detection
   * of user-modified parameters
   */
  // By default set System Information to output on initial
  params.set<MultiMooseEnum>("output_system_information_on", /*quiet_mode=*/true) = "initial";

  // Change the default behavior of 'output_on' to included nonlinear iterations and failed timesteps
  params.set<MultiMooseEnum>("output_on", /*quiet_mode=*/true).push_back("nonlinear failed");

  // By default postprocessors and scalar are only output at the end of a timestep
  params.set<MultiMooseEnum>("output_postprocessors_on", /*quiet_mode=*/true) = "timestep_end";
  params.set<MultiMooseEnum>("output_vector_postprocessors_on", /*quiet_mode=*/true) = "timestep_end";
  params.set<MultiMooseEnum>("output_scalars_on", /*quiet_mode=*/true) = "timestep_end";

  return params;
}

Console::Console(const std::string & name, InputParameters parameters) :
    TableOutput(name, parameters),
    _max_rows(getParam<unsigned int>("max_rows")),
    _fit_mode(getParam<MooseEnum>("fit_mode")),
    _scientific_time(getParam<bool>("scientific_time")),
    _write_file(getParam<bool>("output_file")),
    _write_screen(getParam<bool>("output_screen")),
    _verbose(getParam<bool>("verbose")),
    _old_linear_norm(std::numeric_limits<Real>::max()),
    _old_nonlinear_norm(std::numeric_limits<Real>::max()),
    _perf_log(getParam<bool>("perf_log")),
    _solve_log(isParamValid("solve_log") ? getParam<bool>("solve_log") : _perf_log),
    _setup_log(isParamValid("setup_log") ? getParam<bool>("setup_log") : _perf_log),
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    _libmesh_log(getParam<bool>("libmesh_log")),
#endif
    _setup_log_early(getParam<bool>("setup_log_early")),
    _perf_header(isParamValid("perf_header") ? getParam<bool>("perf_header") : _perf_log),
    _all_variable_norms(getParam<bool>("all_variable_norms")),
    _outlier_variable_norms(getParam<bool>("outlier_variable_norms")),
    _outlier_multiplier(getParam<std::vector<Real> >("outlier_multiplier")),
    _precision(isParamValid("time_precision") ? getParam<unsigned int>("time_precision") : 0),
    _show_output_on_info(getParam<bool>("show_output_on")),
    _timing(_app.getParam<bool>("timing")),
    _console_buffer(_app.getOutputWarehouse().consoleBuffer())
{
  // Apply the special common console flags (print_...)
  ActionWarehouse & awh = _app.actionWarehouse();
  Action * common_action = awh.getActionsByName("common_output")[0];
  if (!_pars.paramSetByUser("output_on") && common_action->getParam<bool>("print_linear_residuals"))
    _output_on.push_back("linear");
  if (!_pars.paramSetByUser("perf_log") && common_action->getParam<bool>("print_perf_log"))
  {
    _perf_log = true;
    _solve_log = true;
    _setup_log = true;
  }

  // If --timing was used from the command-line, do nothing, all logs are enabled
  if (!_timing)
  {
    // Disable performance logging (all log input options must be false)
    if (!_perf_log && !_setup_log && !_solve_log && !_perf_header && !_setup_log_early)
    {
      Moose::perf_log.disable_logging();
      Moose::setup_perf_log.disable_logging();
    }

    // Disable libMesh log
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    if (!_libmesh_log)
      libMesh::perflog.disable_logging();
#endif
  }

  // Set output coloring
  if (Moose::_color_console)
  {
    char * term_env = getenv("TERM");
    if (term_env)
    {
      std::string term(term_env);
      if (term != "xterm-256color" && term != "xterm")
        Moose::_color_console = false;
    }
  }

  // If --show-outputs is used, enable it
  if (_app.getParam<bool>("show_outputs"))
    _show_output_on_info = true;
}

Console::~Console()
{
  // Write the libMesh performance log header
  if (_perf_header)
    write(Moose::perf_log.get_info_header(), false);

  // Write the solve log (Moose Test Performance)
  if (_solve_log)
    write(Moose::perf_log.get_perf_info(), false);

  // Write the setup log (Setup Performance)
  if (_setup_log)
    write(Moose::setup_perf_log.get_perf_info(), false);

  // Write the libMesh log
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
  if (_libmesh_log)
    write(libMesh::perflog.get_perf_info(), false);
#endif

  // Write the file output stream
  writeStreamToFile();

  /* If --timing was not used disable the logging b/c the destructor of these
   * object does the output, if --timing was used do nothing because all other
   * screen related output was disabled above */
  if (!_timing)
  {
    /* Disable the logs, without this the logs will be printed
       during the destructors of the logs themselves */
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    libMesh::perflog.disable_logging();
#endif
  }
}

void
Console::initialSetup()
{
  // Set the string for multiapp output indenting
  if (_app.getOutputWarehouse().multiappLevel() > 0)
    _multiapp_indent = COLOR_CYAN + _app.name() + ": " + COLOR_DEFAULT;

  // If file output is desired, wipe out the existing file if not recovering
  if (!_app.isRecovering())
    writeStreamToFile(false);

  // Enable verbose output if Executioner has it enabled
  if (_app.getExecutioner()->isParamValid("verbose") && _app.getExecutioner()->getParam<bool>("verbose"))
  {
    _verbose = true;
    _pars.set<bool>("verbose") = true;
  }

  // Display a message to indicate the application is running (useful for MultiApps)
  if (_problem_ptr->hasMultiApps() || _app.getOutputWarehouse().multiappLevel() > 0)
    write(std::string("\nRunning App: ") + _app.name() + "\n");

  // Output the performance log early
  if (getParam<bool>("setup_log_early"))
    write(Moose::setup_perf_log.get_perf_info());
}

std::string
Console::filename()
{
  return _file_base + ".txt";
}

void
Console::output(const ExecFlagType & type)
{
  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // Flush the Console buffer, if we don't do this here then the linear/nonlinear residual output
  // may write to the screen prior to buffered text
  _app.getOutputWarehouse().flushConsoleBuffer();

  // Output the system information first; this forces this to be the first item to write by default
  // However, 'output_system_information_on' still operates correctly, so it may be changed by the user
  if (shouldOutput("system_information", type))
    outputSystemInformation();

  // Write the input
  if (shouldOutput("input", type))
    outputInput();

  // Write the timestep information ("Time Step 0 ..."), this is may be controlled with "execute_on"
  if (type == EXEC_TIMESTEP_BEGIN || (type == EXEC_INITIAL && _output_on.contains(EXEC_INITIAL)))
    writeTimestepInformation();

  // Print Non-linear Residual (control with "execute_on")
  if (type == EXEC_NONLINEAR && _output_on.contains(EXEC_NONLINEAR))
  {
    if (_write_screen)
      Moose::out << _multiapp_indent << std::setw(2) << _nonlinear_iter << " Nonlinear |R| = " << outputNorm(_old_nonlinear_norm, _norm) << std::endl;

    if (_write_file)
      _file_output_stream << std::setw(2) << _nonlinear_iter << " Nonlinear |R| = " << std::scientific << _norm << std::endl;
  }

  // Print Linear Residual (control with "execute_on")
  else if (type == EXEC_LINEAR && _output_on.contains(EXEC_LINEAR))
  {
    if (_write_screen)
      Moose::out << _multiapp_indent << std::setw(7) << _linear_iter << " Linear |R| = " <<  outputNorm(_old_linear_norm, _norm) << std::endl;

    if (_write_file)
      _file_output_stream << std::setw(7) << _linear_iter << std::scientific << " Linear |R| = " << std::scientific << _norm << std::endl;
  }

  // Write variable norms
  else if (type == EXEC_TIMESTEP_END)
    writeVariableNorms();

  // Write Postprocessors and Scalars
  if (shouldOutput("postprocessors", type))
    outputPostprocessors();

  if (shouldOutput("scalars", type))
    outputScalarVariables();

  // Write the file
  writeStreamToFile();
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

  // Write contents of file output stream and close the file
  output << _file_output_stream.str();
  output.close();

  // Clear the file output stream
  _file_output_stream.str("");
}

void
Console::writeTimestepInformation()
{
  // Stream to build the time step information
  std::stringstream oss;

  // Write timestep data for transient executioners
  if (_transient)
  {
    // Get the length of the time step string
    std::ostringstream time_step_string;
    time_step_string << timeStep();
    unsigned int n = time_step_string.str().size();
    if (n < 2)
      n = 2;

    // Write time step and time information
    oss << std::endl << "Time Step " << std::setw(n) << timeStep();

    // Set precision
    if (_precision > 0)
      oss << std::setw(_precision) << std::setprecision(_precision) << std::setfill('0') << std::showpoint;

    // Show scientific notation
    if (_scientific_time)
      oss << std::scientific;

    // Print the time
    oss << ", time = " << time() << std::endl;

    // Show old time information, if desired
    if (_verbose)
      oss << std::right << std::setw(21) << std::setfill(' ') << "old time = " << std::left << timeOld() << '\n';

    // Show the time delta information
    oss << std::right << std::setw(21) << std::setfill(' ') << "dt = "<< std::left << dt() << '\n';

    // Show the old time delta information, if desired
    if (_verbose)
      oss << std::right << std::setw(21) << std::setfill(' ') << "old dt = " << _dt_old << '\n';
  }

  // Output to the screen
  write(oss.str());
}

void
Console::writeVariableNorms()
{
  // If all_variable_norms is true, then so should outlier printing
  if (_all_variable_norms)
    _outlier_variable_norms = true;

  // Flag set when header prints
  bool header = false;

  // String stream for variable norm information
  std::ostringstream oss;

  // Get a references to the NonlinearSystem and libMesh system
  NonlinearSystem & nl = _problem_ptr->getNonlinearSystem();
  TransientNonlinearImplicitSystem & sys = nl.sys();

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
    Real var_norm = sys.calculate_norm(*sys.rhs, i, DISCRETE_L2);
    var_norm *= var_norm; // use the norm squared
    std::string var_name = sys.variable_name(i);

    // Outlier if the variable norm is greater than twice (default) of the average norm
    if (_outlier_variable_norms && (var_norm > _outlier_multiplier[1] * avg_norm) )
    {
      // Print the header
      if (!header)
      {
        oss << "\nOutlier Variable Residual Norms:\n";
        header = true;
      }

      // Set the color, RED if the variable norm is 0.8 (default) of the total norm
      std::string color = COLOR_YELLOW;
      if (_outlier_variable_norms && (var_norm > _outlier_multiplier[0] * avg_norm * n_vars) )
        color = COLOR_RED;

      // Display the residual
      oss << "  " << var_name << ": " << std::scientific << color << std::sqrt(var_norm) << COLOR_DEFAULT << '\n';
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
      oss << "  " << var_name << ": " << std::scientific << COLOR_GREEN << std::sqrt(var_norm) << COLOR_DEFAULT << '\n';
    }
  }

  // Update the output streams
  write(oss.str());
}

// Quick helper to output the norm in color
std::string
Console::outputNorm(Real old_norm, Real norm)
{
  std::string color = COLOR_GREEN;

  // Red if the residual went up...
  if (norm > old_norm)
    color = COLOR_RED;
  // Yellow if change is less than 5%
  else if ((old_norm - norm) / old_norm <= 0.05)
    color = COLOR_YELLOW;

  std::stringstream oss;
  oss << std::scientific << color << norm << COLOR_DEFAULT;

  return oss.str();
}


// Method for stringstream formatting
void
Console::insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr)
{
   if (curr - begin > _line_length)
   {
     oss << "\n";
     begin = oss.tellp();
     oss << std::setw(_field_width + 2) << "";  // "{ "
   }
}

void
Console::outputInput()
{
  if (!_write_screen && !_write_file)
    return;

  std::ostringstream oss;
  oss << "--- " << _app.getInputFileName() << " ------------------------------------------------------";
  _app.actionWarehouse().printInputFile(oss);
  oss << "\n";

  if (_write_screen)
    Moose::out << oss.str() << std::endl;

  if (_write_file)
    _file_output_stream << oss.str() << std::endl;
}

void
Console::outputPostprocessors()
{
  TableOutput::outputPostprocessors();

  if (!_postprocessor_table.empty())
  {
    std::stringstream oss;
    oss << "\nPostprocessor Values:\n";
    _postprocessor_table.printTable(oss, _max_rows, _fit_mode);
    oss << std::endl;
    write(oss.str());
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
      _scalar_table.printTable(oss, _max_rows, _fit_mode);
    oss << std::endl;
    write(oss.str());
  }
}


void
Console::indentMessage(std::string & message)
{
  // Indent all lines after the first
  pcrecpp::RE re("\n(?!\\Z)");
  re.GlobalReplace(std::string("\n") + _multiapp_indent, &message);

  // Prepend indent string at the front of the message
  message = _multiapp_indent + message;
}

void
Console::write(std::string message, bool indent)
{
  // Do nothing if the message is empty, writing empty strings messes with multiapp indenting
  if (message.empty())
    return;

  // Write the message to file
  if (_write_file)
    _file_output_stream << message << std::endl;

  // Apply MultiApp indenting
  if (indent)
    indentMessage(message);

  // Write message to the screen
  if (_write_screen)
    Moose::out << message;
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
Console::outputSystemInformation()
{
  // Don't build this information if nothing is to be written
  if (!_write_screen && !_write_file)
    return;

  std::stringstream oss;

  // Framework information
  if (_app.getSystemInfo() != NULL)
    oss << _app.getSystemInfo()->getInfo();

  if (_problem_ptr->legacyUoAuxComputation() || _problem_ptr->legacyUoInitialization())
  {
    oss << "LEGACY MODES ENABLED:\n";
    if (_problem_ptr->legacyUoAuxComputation())
      oss << "  Computing EXEC_LINEAR AuxKernel types when any UserObject type is executed.\n";
    if (_problem_ptr->legacyUoInitialization())
      oss << "  Computing all UserObjects during initial setup.\n";
  }

  oss << std::left << '\n'
      << "Parallelism:\n"
      << std::setw(_field_width) << "  Num Processors: " << static_cast<std::size_t>(n_processors()) << '\n'
      << std::setw(_field_width) << "  Num Threads: " << static_cast<std::size_t>(n_threads()) << '\n'
      << '\n';

  MooseMesh & moose_mesh = _problem_ptr->mesh();
  MeshBase & mesh = moose_mesh.getMesh();
  oss << "Mesh: " << '\n'
      << std::setw(_field_width) << "  Distribution: " << (moose_mesh.isParallelMesh() ? "parallel" : "serial")
      << (moose_mesh.isDistributionForced() ? " (forced) " : "") << '\n'
      << std::setw(_field_width) << "  Mesh Dimension: " << mesh.mesh_dimension() << '\n'
      << std::setw(_field_width) << "  Spatial Dimension: " << mesh.spatial_dimension() << '\n'
      << std::setw(_field_width) << "  Nodes:" << '\n'
      << std::setw(_field_width) << "    Total:" << mesh.n_nodes() << '\n'
      << std::setw(_field_width) << "    Local:" << mesh.n_local_nodes() << '\n'
      << std::setw(_field_width) << "  Elems:" << '\n'
      << std::setw(_field_width) << "    Total:" << mesh.n_elem() << '\n'
      << std::setw(_field_width) << "    Local:" << mesh.n_local_elem() << '\n'
      << std::setw(_field_width) << "  Num Subdomains: "       << static_cast<std::size_t>(mesh.n_subdomains()) << '\n'
      << std::setw(_field_width) << "  Num Partitions: "       << static_cast<std::size_t>(mesh.n_partitions()) << '\n';
  if (n_processors() > 1 && moose_mesh.partitionerName() != "")
    oss << std::setw(_field_width) << "  Partitioner: "       << moose_mesh.partitionerName()
        << (moose_mesh.isPartitionerForced() ? " (forced) " : "")
        << '\n';
  oss << '\n';

  EquationSystems & eq = _problem_ptr->es();
  unsigned int num_systems = eq.n_systems();
  for (unsigned int i=0; i<num_systems; ++i)
  {
    const System & system = eq.get_system(i);
    if (system.system_type() == "TransientNonlinearImplicit")
      oss << "Nonlinear System:" << '\n';
    else if (system.system_type() == "TransientExplicit")
      oss << "Auxiliary System:" << '\n';
    else
      oss << std::setw(_field_width) << system.system_type() << '\n';

    if (system.n_dofs())
    {
      oss << std::setw(_field_width) << "  Num DOFs: " << system.n_dofs() << '\n'
          << std::setw(_field_width) << "  Num Local DOFs: " << system.n_local_dofs() << '\n';

      std::streampos begin_string_pos = oss.tellp();
      std::streampos curr_string_pos = begin_string_pos;
      oss << std::setw(_field_width) << "  Variables: ";
      for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
      {
        const VariableGroup &vg_description (system.variable_group(vg));

        if (vg_description.n_variables() > 1) oss << "{ ";
        for (unsigned int vn=0; vn<vg_description.n_variables(); vn++)
        {
          oss << "\"" << vg_description.name(vn) << "\" ";
          curr_string_pos = oss.tellp();
          insertNewline(oss, begin_string_pos, curr_string_pos);
        }

        if (vg_description.n_variables() > 1) oss << "} ";
      }
      oss << '\n';

      begin_string_pos = oss.tellp();
      curr_string_pos = begin_string_pos;
      oss << std::setw(_field_width) << "  Finite Element Types: ";
#ifndef LIBMESH_ENABLE_INFINITE_ELEMENTS
      for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
      {
        oss << "\""
            << libMesh::Utility::enum_to_string<FEFamily>(system.get_dof_map().variable_group(vg).type().family)
            << "\" ";
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
      }
      oss << '\n';
#else
      for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
      {
        oss << "\""
            << libMesh::Utility::enum_to_string<FEFamily>(system.get_dof_map().variable_group(vg).type().family)
            << "\", \""
            << libMesh::Utility::enum_to_string<FEFamily>(system.get_dof_map().variable_group(vg).type().radial_family)
            << "\" ";
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
      }
      oss << '\n';

      begin_string_pos = oss.tellp();
      curr_string_pos = begin_string_pos;
      oss << std::setw(_field_width) << "  Infinite Element Mapping: ";
      for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
      {
        oss << "\""
            << libMesh::Utility::enum_to_string<InfMapType>(system.get_dof_map().variable_group(vg).type().inf_map)
            << "\" ";
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
      }
      oss << '\n';
#endif

      begin_string_pos = oss.tellp();
      curr_string_pos = begin_string_pos;
      oss << std::setw(_field_width) << "  Approximation Orders: ";
      for (unsigned int vg=0; vg<system.n_variable_groups(); vg++)
      {
#ifndef LIBMESH_ENABLE_INFINITE_ELEMENTS
        oss << "\""
            << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().order)
            << "\" ";
#else
        oss << "\""
            << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().order)
            << "\", \""
            << Utility::enum_to_string<Order>(system.get_dof_map().variable_group(vg).type().radial_order)
            << "\" ";
#endif
        curr_string_pos = oss.tellp();
        insertNewline(oss, begin_string_pos, curr_string_pos);
      }
      oss << "\n\n";
    }
    else
      oss << "   *** EMPTY ***\n\n";
  }

  oss << "Execution Information:\n"
      << std::setw(_field_width) << "  Executioner: " << demangle(typeid(*_app.getExecutioner()).name()) << '\n';

  std::string time_stepper = _app.getExecutioner()->getTimeStepperName();
  if (time_stepper != "")
    oss << std::setw(_field_width) << "  TimeStepper: " << time_stepper << '\n';

  oss << std::setw(_field_width) << "  Solver Mode: " << Moose::stringify<Moose::SolveType>(_problem_ptr->solverParams()._type) << '\n';

  const std::string & pc_desc = _problem_ptr->getPreconditionerDescription();
  if (!pc_desc.empty())
    oss << std::setw(_field_width) << "  Preconditioner: " << pc_desc << '\n';
  oss << '\n';

  // Output information
  if (_show_output_on_info)
  {
    const std::vector<Output *> & outputs = _app.getOutputWarehouse().all();
    oss << "Outputs:\n";
    for (std::vector<Output *>::const_iterator it = outputs.begin(); it != outputs.end(); ++it)
    {
      // Display the "output_on" settings
      const MultiMooseEnum & output_on = (*it)->outputOn();
      oss << "  " << std::setw(_field_width-2) << (*it)->name() <<  "\"" << output_on << "\"\n";

      // Display the advanced "output_on" settings, only if they are different from "output_on"
      if ((*it)->isAdvanced())
      {
        const OutputOnWarehouse & adv_on = (*it)->advancedOutputOn();
        for (std::map<std::string, MultiMooseEnum>::const_iterator adv_it = adv_on.begin(); adv_it != adv_on.end(); ++adv_it)
          if (output_on != adv_it->second)
            oss << "    " << std::setw(_field_width-4) << adv_it->first + ":" <<  "\"" << adv_it->second << "\"\n";
      }
    }
  }

  oss << "\n\n";

  // Output the information
  write(oss.str());
}

void
Console::petscSetupOutput()
{
  char c[] =  {32,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,13,10,124,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,124,13,10,124,32,32,32,32,32,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,32,32,32,32,32,124,13,10,32,92,32,32,32,32,32,32,32,32,32,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,32,32,32,32,32,32,32,32,32,47,13,10,32,32,92,95,95,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,95,95,95,45,45,45,95,95,95,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,95,95,47,13,10,32,32,32,32,32,45,45,45,95,95,95,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,32,92,32,32,32,32,32,32,32,32,32,95,95,95,45,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,45,45,45,95,95,95,32,32,124,32,32,32,32,32,32,32,32,32,124,32,32,95,95,95,45,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,45,45,124,32,32,95,32,32,32,95,32,32,124,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,124,111,124,32,124,111,124,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,45,32,32,32,45,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,32,95,95,95,32,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,45,45,32,32,32,45,45,32,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,32,32,47,92,32,32,32,32,32,47,92,32,32,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,92,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,47,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,47,92,32,32,92,95,95,95,95,95,95,95,95,95,95,95,95,32,47,32,32,47,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,92,32,32,32,32,32,39,95,95,95,39,32,32,32,32,32,47,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,47,92,32,32,32,32,32,92,32,45,45,95,95,45,45,45,95,95,45,45,32,47,32,32,32,32,32,47,92,13,10,32,32,32,32,32,32,32,32,32,47,32,32,92,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,47,32,32,92,13,10,32,32,32,32,32,32,32,32,47,32,32,32,47,32,32,32,32,32,32,32,77,46,79,46,79,46,83,46,69,32,32,32,32,32,32,32,92,32,32,32,92,13,10,32,32,32,32,32,32,32,47,32,32,32,124,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,92,13,10,32,32,32,32,32,32,124,32,32,32,32,124,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,124,32,32,32,32,124,13,10,32,32,32,32,32,32,32,92,32,32,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,47,13,10,32,32,32,32,32,32,32,32,32,92,92,32,92,95,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,95,47,32,47,47,13,10,32,32,32,32,32,32,32,32,32,32,32,45,45,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,45,45,45,95,95,95,95,95,45,45,45,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,124,32,32,32,124,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,124,32,32,32,124,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,86,32,32,32,32,32,92,32,47,32,32,32,32,86,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,95,124,95,95,95,95,95,124,32,124,95,95,95,95,124,95,95,124};
  Moose::out << std::string(c) << std::endl << std::endl;
}
