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
#include "MooseApp.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "PetscSupport.h"
#include "Conversion.h"

#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"

const unsigned short FIELD_WIDTH = 25;
const unsigned short LINE_LENGTH = 100;

template<>
InputParameters validParams<MooseApp>()
{
  InputParameters params;

  params.addCommandLineParam<std::string>("input_file", "-i <input_file>", "Specify an input file");
  params.addCommandLineParam<std::string>("mesh_only", "--mesh-only", "Setup and Output the input mesh only.");

  params.addCommandLineParam<bool>("show_input", "--show-input", "Shows the parsed input file before running the simulation.");

  params.addCommandLineParam<bool>("help", "-h --help", "Displays CLI usage statement.");

  params.addCommandLineParam<std::string>("dump", "--dump [search_string]", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<std::string>("yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<bool>("syntax", "--syntax", "Dumps the associated Action syntax paths ONLY");

  params.addCommandLineParam<unsigned int>("n_threads", "--n-threads=<n>", 1, "Runs the specified number of threads (Intel TBB) per process");

  params.addCommandLineParam<bool>("warn_unused", "-w --warn-unused", "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused", "-e --error-unused", "Error when encountering unused input file options");
  params.addCommandLineParam<bool>("error_override", "-o --error-override", "Error when encountering overridden or parameters supplied multiple times");

  params.addCommandLineParam<bool>("parallel_mesh", "--parallel-mesh", "The libMesh Mesh underlying MooseMesh should always be a ParallelMesh");

  params.addCommandLineParam<unsigned int>("refinements", "-r <n>", 0, "Specify additional initial uniform refinements for automatic scaling");

  params.addCommandLineParam<std::string>("recover", "--recover [file_base]", "Continue the calculation.  If file_base is ommitted then the most recent recovery file will be utilized");

  params.addCommandLineParam<bool>("half_transient", "--half-transient", "When true the simulation will only run half of its specified transient (ie half the timesteps).  This is useful for testing recovery and restart");

  params.addCommandLineParam<bool>("trap_fpe", "--trap-fpe", "Enable Floating Point Exception handling in critical sections of code.  This is enabled automatically in DEBUG mode");

  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char**>("_argv");

  return params;
}

// Free function for stringstream formating
void insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr)
{
  if (curr - begin > LINE_LENGTH)
  {
    oss << "\n";
    begin = oss.tellp();
    oss << std::setw(FIELD_WIDTH + 2) << "";  // "{ "
  }
}

MooseApp::MooseApp(const std::string & name, InputParameters parameters):
    _name(name),
    _pars(parameters),
    _output_position_set(false),
    _start_time_set(false),
    _start_time(0.0),
    _global_time_offset(0.0),
    _command_line(NULL),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _parser(*this, _action_warehouse),
    _executioner(NULL),
    _sys_info(NULL),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _initial_from_file(false),
    _parallel_mesh_on_command_line(false),
    _recover(false),
    _half_transient(false)
{
  if(isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char**>("_argv");

    _sys_info = new SystemInfo(argc, argv);
    _command_line = new CommandLine(argc, argv);
    _command_line->addCommandLineOptionsFromParams(_pars);
  }
}

MooseApp::~MooseApp()
{
  delete _command_line;
  delete _sys_info;
  delete _executioner;
  _action_warehouse.clear();
}

void
MooseApp::setupOptions()
{
  if (isParamValid("error_unused"))
    setCheckUnusedFlag(true);
  else if (isParamValid("warn_unused"))
    setCheckUnusedFlag(false);

  if (isParamValid("error_override"))
    setErrorOverridden();

  if (isParamValid("parallel_mesh"))
    _parallel_mesh_on_command_line = true;

  if(isParamValid("half_transient"))
    _half_transient = true;

  if (isParamValid("trap_fpe"))
    // Seting Global Variable
    Moose::__trap_fpe = true;

  if (isParamValid("help"))
  {
    _command_line->printUsage();
    _ready_to_exit = true;
  }
  else if (isParamValid("dump"))
  {
    _parser.initSyntaxFormatter(Parser::INPUT_FILE, true);

    // Get command line argument following --dump on command line
    std::string dump_following_arg = getParam<std::string>("dump");

    // If the argument following --dump is non-existent or begins with
    // a dash, call buildFullTree() with an empty string, otherwise
    // pass the argument following --dump.
    if (dump_following_arg.empty() || (dump_following_arg.find('-') == 0))
      _parser.buildFullTree("");
    else
      _parser.buildFullTree(dump_following_arg);

    _ready_to_exit = true;
  }
  else if (isParamValid("yaml"))
  {
    _parser.initSyntaxFormatter(Parser::YAML, true);

    // Get command line argument following --yaml on command line
    std::string yaml_following_arg = getParam<std::string>("yaml");

    // If the argument following --yaml is non-existent or begins with
    // a dash, call buildFullTree() with an empty string, otherwise
    // pass the argument following --yaml.
    if (yaml_following_arg.empty() || (yaml_following_arg.find('-') == 0))
      _parser.buildFullTree("");
    else
      _parser.buildFullTree(yaml_following_arg);

    _ready_to_exit = true;
  }
  else if (isParamValid("syntax"))
  {
    std::multimap<std::string, Syntax::ActionInfo> syntax = _syntax.getAssociatedActions();
    Moose::out << "**START SYNTAX DATA**\n";
    for (std::multimap<std::string, Syntax::ActionInfo>::iterator it = syntax.begin(); it != syntax.end(); ++it)
    {
      Moose::out << it->first << "\n";
    }
    Moose::out << "**END SYNTAX DATA**\n" << std::endl;
    _ready_to_exit = true;
  }
  else if(_input_filename != "") // They already specified an input filename
  {
    _parser.parse(_input_filename);
    _action_warehouse.build();
    return;
  }
  else if (isParamValid("input_file"))
  {
    if(isParamValid("recover"))
    {
      _recover = true;

      // Get command line argument following --recover on command line
      std::string recover_following_arg = getParam<std::string>("recover");

      // If the argument following --recover is non-existent or begins with
      // a dash then we are going to eventually find the newest recovery file to use
      if (!(recover_following_arg.empty() || (recover_following_arg.find('-') == 0)))
        _recover_base = recover_following_arg;
    }

    _input_filename = getParam<std::string>("input_file");
    _parser.parse(_input_filename);
    _action_warehouse.build();
  }
  else
  {
    _command_line->printUsage();
    _ready_to_exit = true;
  }
}

void
MooseApp::setInputFileName(std::string input_filename)
{
  _input_filename = input_filename;
}

void
MooseApp::runInputFile()
{
  std::string mesh_file_name;
  if (isParamValid("mesh_only"))
  {
    meshOnly(getParam<std::string>("mesh_only"));
    _ready_to_exit = true;
  }

  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  // Print the input file syntax if requested
  if (isParamValid("show_input"))
  {
    _action_warehouse.printInputFile(Moose::out);
  }

  _action_warehouse.executeAllActions();
  _executioner = _action_warehouse.executioner();

  // If requested, see if there are unidentified name/value pairs in the input file
  if (isParamValid("error_unused") || _enable_unused_check == ERROR_UNUSED)
  {
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, true);
  }
  else if (isParamValid("warn_unused") || _enable_unused_check == WARN_UNUSED)
  {
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, _enable_unused_check == ERROR_UNUSED);
  }

  if (isParamValid("error_override") || _error_overridden)
    _parser.checkOverriddenParams(true);
  else
    _parser.checkOverriddenParams(false);
}

void
MooseApp::executeExecutioner()
{
  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  // run the simulation
  if (_executioner)
  {
    Moose::PetscSupport::petscSetupOutput(_command_line);

    if (hasLegacyOutput())
    {
      Moose::out << getSysInfo();
      printSimulationInfo(Moose::out);
    }

    _executioner->init();
    _executioner->execute();
  }
  else
    mooseError("No executioner was specified (go fix your input file)");
}

void
MooseApp::meshOnly(std::string mesh_file_name)
{
  /**
   * These actions should be the minimum set necessary to generate and output
   * a Mesh.
   */
  _action_warehouse.executeActionsWithAction("set_global_params");
  _action_warehouse.executeActionsWithAction("setup_mesh");
  _action_warehouse.executeActionsWithAction("prepare_mesh");
  _action_warehouse.executeActionsWithAction("add_mesh_modifier");
  _action_warehouse.executeActionsWithAction("setup_mesh_complete");

  // uniform refinement
  MooseMesh * mesh = _action_warehouse.mesh();
  MeshRefinement mesh_refinement(mesh->getMesh());
  mesh_refinement.uniformly_refine(mesh->uniformRefineLevel());

  // If no argument specified or if the argument following --mesh-only starts
  // with a dash, try to build an output filename based on the input mesh filename.
  if (mesh_file_name.empty() || (mesh_file_name.find('-') == 0))
  {
    mesh_file_name = _parser.getFileName();
    size_t pos = mesh_file_name.find_last_of('.');

    // Default to writing out an ExodusII mesh base on the input filename.
    mesh_file_name = mesh_file_name.substr(0,pos) + "_in.e";
  }

  mesh->getMesh().write(mesh_file_name);

  // Since we are not going to create a problem the mesh
  // will not get cleaned up, so we'll do it here
  delete mesh;
  delete _action_warehouse.displacedMesh();

}

void
MooseApp::setCheckUnusedFlag(bool warn_is_error)
{
  _enable_unused_check = warn_is_error ? ERROR_UNUSED : WARN_UNUSED;
}

void
MooseApp::disableCheckUnusedFlag()
{
  _enable_unused_check = OFF;
}

std::string
MooseApp::getSysInfo()
{
  if(_sys_info)
    return _sys_info->getInfo();
  else
    return "";
}

void
MooseApp::setErrorOverridden()
{
  _error_overridden = true;
}

void
MooseApp::run()
{
  setupOptions();
  runInputFile();
  executeExecutioner();
}

void
MooseApp::setOutputPosition(Point p)
{
  _output_position_set = true;
  _output_position = p;

  if(_executioner)
    _executioner->setOutputPosition(p);
}

std::string
MooseApp::getFileName(bool stripLeadingPath) const
{
  return _parser.getFileName(stripLeadingPath);
}


void
MooseApp::printSimulationInfo(std::ostream & ostream)
{

  std::stringstream oss;

  oss << std::left << '\n'
      << "Parallelism:\n"
      << std::setw(FIELD_WIDTH) << "  Num Processors: "       << static_cast<std::size_t>(libMesh::n_processors()) << '\n'
      << std::setw(FIELD_WIDTH) << "  Num Threads: "         << static_cast<std::size_t>(libMesh::n_threads()) << '\n'
      << '\n';

  MooseMesh *moose_mesh = _action_warehouse.mesh();
  if (moose_mesh)
  {
    MeshBase & mesh = moose_mesh->getMesh();

    oss << std::setw(FIELD_WIDTH) << "Mesh: " << '\n'
        << std::setw(FIELD_WIDTH) << "  Distribution: " << (moose_mesh->isParallelMesh() ? "parallel" : "serial")
        << (moose_mesh->isDistributionForced() ? " (forced) " : "") << '\n'
        << std::setw(FIELD_WIDTH) << "  Mesh Dimension: " << mesh.mesh_dimension() << '\n'
        << std::setw(FIELD_WIDTH) << "  Spatial Dimension: " << mesh.spatial_dimension() << '\n'
        << std::setw(FIELD_WIDTH) << "  Nodes:" << '\n'
        << std::setw(FIELD_WIDTH) << "    Total:" << mesh.n_nodes() << '\n'
        << std::setw(FIELD_WIDTH) << "    Local:" << mesh.n_local_nodes() << '\n'
        << std::setw(FIELD_WIDTH) << "  Elems:" << '\n'
        << std::setw(FIELD_WIDTH) << "    Total:" << mesh.n_elem() << '\n'
        << std::setw(FIELD_WIDTH) << "    Local:" << mesh.n_local_elem() << '\n'
        << std::setw(FIELD_WIDTH) << "  Num Subdomains: "       << static_cast<std::size_t>(mesh.n_subdomains()) << '\n'
        << std::setw(FIELD_WIDTH) << "  Num Partitions: "       << static_cast<std::size_t>(mesh.n_partitions()) << '\n';
    if (libMesh::n_processors() > 1 && moose_mesh->partitionerName() != "")
      oss << std::setw(FIELD_WIDTH) << "  Partitioner: "       << moose_mesh->partitionerName()
          << (moose_mesh->isPartitionerForced() ? " (forced) " : "")
          << '\n';
    oss << '\n';
  }

  FEProblem *problem = _action_warehouse.problem();
  if (problem)
  {
    EquationSystems & eq = _action_warehouse.problem()->es();
    unsigned int num_systems = eq.n_systems();
    for (unsigned int i=0; i<num_systems; ++i)
    {
      const System & system = eq.get_system(i);
      if (system.system_type() == "TransientNonlinearImplicit")
        oss << std::setw(FIELD_WIDTH) << "Nonlinear System:" << '\n';
      else if (system.system_type() == "TransientExplicit")
        oss << std::setw(FIELD_WIDTH) << "Auxiliary System:" << '\n';
      else
        oss << std::setw(FIELD_WIDTH) << system.system_type() << '\n';

      if (system.n_dofs())
      {
        oss << std::setw(FIELD_WIDTH) << "  Num DOFs: " << system.n_dofs() << '\n'
            << std::setw(FIELD_WIDTH) << "  Num Local DOFs: " << system.n_local_dofs() << '\n';

        std::streampos begin_string_pos = oss.tellp();
        std::streampos curr_string_pos = begin_string_pos;
        oss << std::setw(FIELD_WIDTH) << "  Variables: ";
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
        oss << std::setw(FIELD_WIDTH) << "  Finite Element Types: ";
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
        oss << std::setw(FIELD_WIDTH) << "  Infinite Element Mapping: ";
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
        oss << std::setw(FIELD_WIDTH) << "  Approximation Orders: ";
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
        oss << "  *** EMPTY ***\n\n";
    }

    oss << "Execution Information:\n"
        << std::setw(FIELD_WIDTH) << "  Executioner: " << demangle(typeid(*_executioner).name()) << '\n';

    std::string time_stepper = _executioner->getTimeStepper();
    if (time_stepper != "")
      oss << std::setw(FIELD_WIDTH) << "  TimeStepper: " << time_stepper << '\n';

    oss << std::setw(FIELD_WIDTH) << "  Solver Mode: " << Moose::stringify<Moose::SolveType>(_action_warehouse.problem()->solverParams()._type) << '\n';
    oss << '\n';
  }

  ostream << oss.str();
  ostream.flush();
}
