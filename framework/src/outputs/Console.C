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
  params += validParams<FileOutputBase>();

  // Screen and file output toggles
  params.addParam<bool>("screen", true, "Output to the screen");
  params.addParam<bool>("file", false, "Output to the file");

  // Table fitting options
  params.addParam<unsigned int>("max_rows", 15, "The maximum number of postprocessor/scalar values displayed on screen during a timestep (set to 0 for unlimited)");
  params.addParam<MooseEnum>("fit_mode", pps_fit_mode, "Specifies the wrapping mode for post-processor tables that are printed to the screen (ENVIRONMENT: Read \"PPS_WIDTH\" for desired width, AUTO: Attempt to determine width automatically (serial only), <n>: Desired width");

  // Timestep verbosity
  params.addParam<bool>("verbose", false, "Print detailed diagnostics on timestep calculation");

  // Basic table output controls
  params.addParam<bool>("use_color", true, "If true, color will be added to the output");
  params.addParam<bool>("linear_residuals", true, "Specifies whether the linear residuals are printed during the solve");
  params.addParam<bool>("nonlinear_residuals", true, "Specifies whether the nonlinear residuals are printed during the solve");

  // Performance Logging
  params.addParam<bool>("perf_log", false, "If true, all performance logs will be printed. The individual log settings will override this option.");
  params.addParam<bool>("setup_log_early", false, "Specifies whether or not the Setup Performance log should be printed before the first time step.  It will still be printed at the end if ""perf_log"" is also enabled and likewise disabled if ""perf_log"" is false");
  params.addParam<bool>("setup_log", "Toggles the printing of the 'Setup Performance' log");
  params.addParam<bool>("solve_log", "Toggles the printing of the 'Moose Test Performance' log");
  params.addParam<bool>("perf_header", true, "Print the libMesh performance log header (requires that 'perf_log = true')");

  // Advanced group
  params.addParamNamesToGroup("max_rows fit_node verbose", "Advanced");

  // Performance log group
  params.addParamNamesToGroup("perf_log setup_log_early setup_log solve_log perf_header", "Performance Log");

  return params;
}

Console::Console(const std::string & name, InputParameters parameters) :
    TableOutputBase(name, parameters),
    FileOutputBase(name, parameters),
    _max_rows(getParam<unsigned int>("max_rows")),
    _fit_mode(getParam<MooseEnum>("fit_mode")),
    _use_color(false),
    _print_linear(getParam<bool>("linear_residuals")),
    _print_nonlinear(getParam<bool>("nonlinear_residuals")),
    _write_file(getParam<bool>("file")),
    _write_screen(getParam<bool>("screen")),
    _verbose(getParam<bool>("verbose")),
    _old_linear_norm(std::numeric_limits<Real>::max()),
    _old_nonlinear_norm(std::numeric_limits<Real>::max()),
    _perf_log(getParam<bool>("perf_log")),
    _solve_log(isParamValid("solve_log") ? getParam<bool>("solve_log") : _perf_log),
    _setup_log(isParamValid("setup_log") ? getParam<bool>("setup_log") : _perf_log),
    _setup_log_early(getParam<bool>("setup_log_early")),
    _perf_header(isParamValid("perf_header") ? getParam<bool>("perf_header") : _perf_log)
{

  // Disable performance logging (all log input options must be false)
  if (!_perf_log && !_setup_log && !_solve_log && !_perf_header && !_setup_log_early)
  {
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
  }

  // Set output coloring
  if(getParam<bool>("use_color"))
  {
    char * term_env = getenv("TERM");
    if(term_env)
    {
      std::string term(term_env);
      if(term == "xterm-256color" || term == "xterm")
        _use_color = true;
    }
  }

  // If file output is desired, wipe out an existing file
  if (_write_file)
    writeStream(false);

  // Call the PETSc setup function, this sets up the monitor functions for the upcoming timestep
  petscSetup();
}

Console::~Console()
{

  // Write the libMesh performance log header
  if (_perf_header)
  {
    if (_write_screen)
      Moose::out << Moose::perf_log.get_info_header();

    if (_write_file)
      _file_output_stream << Moose::perf_log.get_info_header();
  }

  // Write the solve log (Moose Test Performance)
  if (_solve_log)
  {
    if (_write_screen)
      Moose::out << Moose::perf_log.get_perf_info();
    if (_write_file)
      _file_output_stream << Moose::perf_log.get_perf_info();
  }

  // Write the setup log (Setup Performance)
  if (_setup_log)
  {
    if (_write_screen)
      Moose::out << Moose::setup_perf_log.get_perf_info();
    if (_write_file)
      _file_output_stream << Moose::setup_perf_log.get_perf_info();
  }

  // Write the file output stream
  if (_write_file)
    writeStream();

  /// If an 'Output' block exists do not disable the logging b/c it
  /// will disable the printing of the log as it was in the old system
  /// \todo{Remove this if when the old system is removed}
  if (!_app.hasLegacyOutput())
  {
    /* Disable the logs, without this the logs will be printed
       during the destructors of the logs themselves*/
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
  }
}

void
Console::initialSetup()
{
  // Output the performance log early
  if (getParam<bool>("setup_log_early"))
  {
    if (_write_screen)
      Moose::out << Moose::setup_perf_log.get_perf_info() << std::endl;

    if (_write_file)
      _file_output_stream << Moose::setup_perf_log.get_perf_info() << std::endl;
  }

  // Ouput the system information
  if (_system_information)
    outputSystemInformation();
}

void
Console::timestepSetup()
{
  // Do nothing if the problem is transient
  if (!_transient)
    return;

  // Stream to build the time step information
  std::stringstream oss;

  // Get the length of the time step string
  std::ostringstream time_step_string;
  time_step_string << _t_step;
  unsigned int n = time_step_string.str().size();
  if (n < 2)
    n = 2;

  // Write time step and time information
  oss << std::endl <<  "Time Step " << std::setw(n) << _t_step
      << ", time = " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint << std::left << _time
      << std::endl;

  // Show old time information, if desired
  if (_verbose)
    oss << std::setw(n) << "          old time = " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint << std::left << _time_old << std::endl;

  // Show the time delta information
  oss << std::setw(2) << "                dt = " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint << std::left << _dt << std::endl;

  // Show the old time delta information, if desired
  if (_verbose)
    oss << std::setw(2) << "            old dt = " << std::setw(9) << std::setprecision(6) << std::setfill('0') << std::showpoint << std::left << _dt_old << std::endl;

  // Output to the screen
  if (_write_screen)
    Moose::out << oss.str();

  // Output to the file
  if (_write_file)
    _file_output_stream << oss.str();
}

std::string
Console::filename()
{
  return _file_base + ".txt";
}

void
Console::writeStream(bool append)
{
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
Console::output()
{
  // Call the base class output function
  OutputBase::output();

  // Write the file
  if (_write_file)
    writeStream();

  // Prepare the PETSc monitor functions for the next timestep
  petscSetup();
}

void
Console::linearMonitor(PetscInt & its, PetscReal & norm)
{
  if (_write_screen)
    Moose::out << std::setw(7) << its << " Linear |R| = " << std::scientific << outputNorm(_old_linear_norm, norm) << std::endl;

  if (_write_file)
    _file_output_stream << std::setw(7) << its << std::scientific << " Linear |R| = " << std::scientific << norm << std::endl;
}

void
Console::nonlinearMonitor(PetscInt & its, PetscReal & norm)
{
  if (_write_screen)
    Moose::out << std::setw(2) << its << " Nonlinear |R| = " << outputNorm(_old_nonlinear_norm, norm) << std::endl;

  if (_write_file)
    _file_output_stream << std::setw(2) << its << " Nonlinear |R| = " << std::scientific << norm << std::endl;
}

// Quick helper to output the norm in color
std::string Console::outputNorm(Real old_norm, Real norm)
{
  std::string color(COLOR_DEFAULT);

  // Use color
  if (_use_color)
  {
    // Red if the residual went up...
    if (norm > old_norm)
      color = RED;
    // Yellow if change is less than 5%
    else if ((old_norm - norm) / old_norm <= 0.05)
      color = YELLOW;
    // Green if change is more than 5%
    else
      color = GREEN;
    // Return the colored text
    return MooseUtils::colorText<Real>(color, norm);
  }

  // Return the text without color codes
  else
    return Moose::stringify<Real>(norm);
}


// Free function for stringstream formating
void Console::insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr)
{
   if (curr - begin > _line_length)
   {
     oss << "\n";
     begin = oss.tellp();
     oss << std::setw(_field_width + 2) << "";  // "{ "
   }
}

void
Console::petscSetup()
{
  // Enable/disable the printing of linear residuals
  if (_print_nonlinear)
  {
    Moose::PetscSupport::petscPrintNonlinearResiduals(_problem_ptr, this);

    // Enable/disable the printing of nonlinear residuals
    if (_print_linear)
      Moose::PetscSupport::petscPrintLinearResiduals(_problem_ptr, this);
  }
}

void
Console::outputPostprocessors()
{
  TableOutputBase::outputPostprocessors();

  if (!_postprocessor_table.empty())
  {
    std::stringstream oss;
    oss << "\nPostprocessor Values:\n";
    _postprocessor_table.printTable(oss, _max_rows, _fit_mode);
    oss << std::endl;

    if (_write_screen)
      Moose::out << oss.str();

    if (_write_file)
      _file_output_stream << oss.str();
  }
}

void
Console::outputScalarVariables()
{
  TableOutputBase::outputScalarVariables();

  if (!_scalar_table.empty())
  {
    std::stringstream oss;
    oss << "\nScalar Variable Values:\n";
    _scalar_table.printTable(oss, _max_rows, _fit_mode);
    oss << std::endl;

    if (_write_screen)
      Moose::out << oss.str();

    if (_write_file)
      _file_output_stream << oss.str();
  }
}


void
Console::outputSystemInformation()
{

  std::stringstream oss;

  // Framework information
  oss << _app.getSysInfo();


  oss << std::left << "\n"
      << "Parallelism:\n"
      << std::setw(_field_width) << "  Num Processors: "       << static_cast<std::size_t>(libMesh::n_processors()) << '\n'
      << std::setw(_field_width) << "  Num Threads: "         << static_cast<std::size_t>(libMesh::n_threads()) << '\n'
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
  if (libMesh::n_processors() > 1 && moose_mesh.partitionerName() != "")
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
      oss <<  "Nonlinear System:" << '\n';
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

  std::string time_stepper = _app.getExecutioner()->getTimeStepper();
  if (time_stepper != "")
    oss << std::setw(_field_width) << "  TimeStepper: " << time_stepper << '\n';

  oss << std::setw(_field_width) << "  Solver Mode: " << Moose::stringify<Moose::SolveType>(_problem_ptr->solverParams()._type) << '\n';
  oss << '\n';

  oss.flush();

  // Output the information
  if (_write_screen)
    Moose::out << oss.str();

  if (_write_file)
    _file_output_stream << oss.str();
}

void
Console::petscSetupOutput()
{
char c[] =  {32,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,13,10,124,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,124,13,10,124,32,32,32,32,32,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,32,32,32,32,32,124,13,10,32,92,32,32,32,32,32,32,32,32,32,32,32,32,92,95,47,94,92,32,32,32,32,32,32,32,32,32,32,32,47,94,92,95,47,32,32,32,32,32,32,32,32,32,32,32,32,47,13,10,32,32,92,95,95,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,95,95,95,45,45,45,95,95,95,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,95,95,47,13,10,32,32,32,32,32,45,45,45,95,95,95,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,32,92,32,32,32,32,32,32,32,32,32,95,95,95,45,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,45,45,45,95,95,95,32,32,124,32,32,32,32,32,32,32,32,32,124,32,32,95,95,95,45,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,45,45,124,32,32,95,32,32,32,95,32,32,124,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,124,111,124,32,124,111,124,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,45,32,32,32,45,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,32,95,95,95,32,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,45,45,32,32,32,45,45,32,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,32,32,47,92,32,32,32,32,32,47,92,32,32,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,92,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,47,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,47,92,32,32,92,95,95,95,95,95,95,95,95,95,95,95,95,32,47,32,32,47,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,92,32,32,32,32,32,39,95,95,95,39,32,32,32,32,32,47,32,32,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,47,92,32,32,32,32,32,92,32,45,45,95,95,45,45,45,95,95,45,45,32,47,32,32,32,32,32,47,92,13,10,32,32,32,32,32,32,32,32,32,47,32,32,92,47,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,92,47,32,32,92,13,10,32,32,32,32,32,32,32,32,47,32,32,32,47,32,32,32,32,32,32,32,77,46,79,46,79,46,83,46,69,32,32,32,32,32,32,32,92,32,32,32,92,13,10,32,32,32,32,32,32,32,47,32,32,32,124,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,92,13,10,32,32,32,32,32,32,124,32,32,32,32,124,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,124,32,32,32,32,124,13,10,32,32,32,32,32,32,32,92,32,32,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,32,32,47,13,10,32,32,32,32,32,32,32,32,32,92,92,32,92,95,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,95,47,32,47,47,13,10,32,32,32,32,32,32,32,32,32,32,32,45,45,32,32,92,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,32,45,45,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,45,45,45,95,95,95,95,95,45,45,45,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,124,32,32,32,124,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,32,32,32,32,32,124,32,32,32,124,32,32,32,32,32,124,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,47,32,86,32,32,32,32,32,92,32,47,32,32,32,32,86,32,32,92,13,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,124,95,124,95,95,95,95,95,124,32,124,95,95,95,95,124,95,95,124};
Moose::out << std::string(c) << std::endl << std::endl;
}
