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
#include "CommandLine.h"

// libMesh includes
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<MooseApp>()
{
  InputParameters params;

  params.addCommandLineParam<std::string>("input_file", "-i <input_file>", "Specify an input file");
  params.addCommandLineParam<std::string>("mesh_only", "--mesh-only", "Setup and Output the input mesh only.");

  params.addCommandLineParam<bool>("show_input", "--show-input", false, "Shows the parsed input file before running the simulation.");
  params.addCommandLineParam<bool>("show_outputs", "--show-outputs", false, "Shows the output execution time information.");

  params.addCommandLineParam<bool>("no_color", "--no-color", false, "Disable coloring of all Console outputs.");

  params.addCommandLineParam<bool>("help", "-h --help", false, "Displays CLI usage statement.");

  params.addCommandLineParam<std::string>("dump", "--dump [search_string]", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<std::string>("yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<bool>("syntax", "--syntax", false, "Dumps the associated Action syntax paths ONLY");
  params.addCommandLineParam<bool>("check_input", "--check-input", false, "Check the input file (i.e. requires -i <filename>) and quit.");

  params.addCommandLineParam<unsigned int>("n_threads", "--n-threads=<n>", 1, "Runs the specified number of threads per process");

  params.addCommandLineParam<bool>("warn_unused", "-w --warn-unused", false, "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused", "-e --error-unused", false, "Error when encountering unused input file options");
  params.addCommandLineParam<bool>("error_override", "-o --error-override", false, "Error when encountering overridden or parameters supplied multiple times");

  params.addCommandLineParam<bool>("parallel_mesh", "--parallel-mesh", false, "The libMesh Mesh underlying MooseMesh should always be a ParallelMesh");

  params.addCommandLineParam<unsigned int>("refinements", "-r <n>", 0, "Specify additional initial uniform refinements for automatic scaling");

  params.addCommandLineParam<std::string>("recover", "--recover [file_base]", "Continue the calculation.  If file_base is omitted then the most recent recovery file will be utilized");

  params.addCommandLineParam<bool>("half_transient", "--half-transient", false, "When true the simulation will only run half of its specified transient (ie half the timesteps).  This is useful for testing recovery and restart");

  // No default on these two options, they must not both be valid
  params.addCommandLineParam<bool>("trap_fpe", "--trap-fpe", "Enable Floating Point Exception handling in critical sections of code.  This is enabled automatically in DEBUG mode");
  params.addCommandLineParam<bool>("no_trap_fpe", "--no-trap-fpe", "Disable Floating Point Exception handling in critical sections of code when using DEBUG mode.");

  params.addCommandLineParam<bool>("error", "--error", false, "Turn all warnings into errors");

  params.addCommandLineParam<bool>("timing", "-t --timing", false, "Enable all performance logging for timing purposes. This will disable all screen output of performance logs for all Console objects.");

  // Legacy Flags
  params.addParam<bool>("use_legacy_uo_aux_computation", true, "Set to true to have MOOSE recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");
  params.addParam<bool>("use_legacy_uo_initialization", true, "Set to true to have MOOSE compute all UserObjects and Postprocessors during the initial setup phase of the problem recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");

  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char**>("_argv");
  params.addPrivateParam<MooseSharedPointer<CommandLine> >("_command_line");
  params.addPrivateParam<MooseSharedPointer<Parallel::Communicator> >("_comm");

  return params;
}

// Free function for removing cli flags
bool isFlag(const std::string s)
{
  return s.length() && s[0] == '-';
}

MooseApp::MooseApp(const std::string & name, InputParameters parameters) :
    ParallelObject(*parameters.get<MooseSharedPointer<Parallel::Communicator> >("_comm")), // Can't call getParam() before pars is set
    _name(name),
    _pars(parameters),
    _comm(getParam<MooseSharedPointer<Parallel::Communicator> >("_comm")),
    _output_position_set(false),
    _start_time_set(false),
    _start_time(0.0),
    _global_time_offset(0.0),
    _alternate_output_warehouse(NULL),
    _output_warehouse(new OutputWarehouse),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _parser(*this, _action_warehouse),
    _use_nonlinear(true),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _initial_from_file(false),
    _parallel_mesh_on_command_line(false),
    _recover(false),
    _restart(false),
    _half_transient(false),
    _legacy_uo_aux_computation_default(getParam<bool>("use_legacy_uo_aux_computation")),
    _legacy_uo_initialization_default(getParam<bool>("use_legacy_uo_initialization")),
    _check_input(getParam<bool>("check_input"))
{
  if (isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char**>("_argv");

    _sys_info = MooseSharedPointer<SystemInfo>(new SystemInfo(argc, argv));
  }
  if (isParamValid("_command_line"))
    _command_line = getParam<MooseSharedPointer<CommandLine> >("_command_line");
  else
    mooseError("Valid CommandLine object required");
}

MooseApp::~MooseApp()
{
  _action_warehouse.clear();

  // MUST be deleted before _comm is destroyed!
  delete _output_warehouse;
}

void
MooseApp::setupOptions()
{
  if (getParam<bool>("error_unused"))
    setCheckUnusedFlag(true);
  else if (getParam<bool>("warn_unused"))
    setCheckUnusedFlag(false);

  if (getParam<bool>("error_override"))
    setErrorOverridden();

  _parallel_mesh_on_command_line = getParam<bool>("parallel_mesh");
  _half_transient = getParam<bool>("half_transient");
  _pars.set<bool>("timing") = getParam<bool>("timing");

  if (isParamValid("trap_fpe") && isParamValid("no_trap_fpe"))
    mooseError("Cannot use both \"--trap-fpe\" and \"--no-trap-fpe\" flags.");
  if (isParamValid("trap_fpe"))
    Moose::_trap_fpe = true;
  else if (isParamValid("no_trap_fpe"))
    Moose::_trap_fpe = false;

  Moose::_warnings_are_errors = getParam<bool>("error");
  Moose::_color_console = !getParam<bool>("no_color");

  if (getParam<bool>("help"))
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
  else if (getParam<bool>("syntax"))
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
  else if (_input_filename != "" || isParamValid("input_file")) // They already specified an input filename
  {
    if (_input_filename == "")
      _input_filename = getParam<std::string>("input_file");

    if (isParamValid("recover"))
    {
      // We need to set the flag manually here since the recover parameter is a string type (takes an optional filename)
      _recover = true;

      // Get command line argument following --recover on command line
      std::string recover_following_arg = getParam<std::string>("recover");

      // If the argument following --recover is non-existent or begins with
      // a dash then we are going to eventually find the newest recovery file to use
      if (!(recover_following_arg.empty() || (recover_following_arg.find('-') == 0)))
        _recover_base = recover_following_arg;
    }

    _parser.parse(_input_filename);
    _action_warehouse.build();
  }
  else
  {
    if (_check_input)
      mooseError("You specified --check-input, but did not provide an input file. Add -i <inputfile> to your command line.");

    _command_line->printUsage();
    _ready_to_exit = true;
  }
}

void
MooseApp::setInputFileName(std::string input_filename)
{
  _input_filename = input_filename;
}

std::string
MooseApp::getOutputFileBase()
{
  return _output_file_base;
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

  _action_warehouse.executeAllActions();
  _executioner = _action_warehouse.executioner();

  bool error_unused = getParam<bool>("error_unused") || _enable_unused_check == ERROR_UNUSED;
  bool warn_unused = getParam<bool>("warn_unused") || _enable_unused_check == WARN_UNUSED;

  if (error_unused || warn_unused)
  {
    // Check the input file parameters
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, error_unused, true);

    // Only check CLI parameters on the main application
    // TODO: Add support for SubApp overrides and checks #4119
    if (_name == "main")
    {
      // Check the CLI parameters
      all_vars = _command_line->getPot()->get_variable_names();
      // Remove flags, they aren't "input" parameters
      all_vars.erase( std::remove_if(all_vars.begin(), all_vars.end(), isFlag), all_vars.end() );

      _parser.checkUnidentifiedParams(all_vars, error_unused, false);
    }
  }

  if (getParam<bool>("error_override") || _error_overridden)
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
#ifdef LIBMESH_HAVE_PETSC
    Moose::PetscSupport::petscSetupOutput(_command_line.get());
#endif
    _executioner->init();
    if (_check_input)
    {
      // Output to stderr, so it is easier for peacock to get the result
      Moose::err << "Syntax OK" << std::endl;
      return;
    }
    _executioner->execute();
  }
  else
    mooseError("No executioner was specified (go fix your input file)");
}

void
MooseApp::meshOnly(std::string mesh_file_name)
{
  /**
   * Thesgete actions should be the minimum set necessary to generate and output
   * a Mesh.
   */
  _action_warehouse.executeActionsWithAction("set_global_params");
  _action_warehouse.executeActionsWithAction("setup_mesh");
  _action_warehouse.executeActionsWithAction("prepare_mesh");
  _action_warehouse.executeActionsWithAction("add_mesh_modifier");
  _action_warehouse.executeActionsWithAction("uniform_refine_mesh");
  _action_warehouse.executeActionsWithAction("setup_mesh_complete");

  MooseSharedPointer<MooseMesh> & mesh = _action_warehouse.mesh();

  // If no argument specified or if the argument following --mesh-only starts
  // with a dash, try to build an output filename based on the input mesh filename.
  if (mesh_file_name.empty() || (mesh_file_name.find('-') == 0))
  {
    mesh_file_name = _parser.getFileName();
    size_t pos = mesh_file_name.find_last_of('.');

    // Default to writing out an ExodusII mesh base on the input filename.
    mesh_file_name = mesh_file_name.substr(0, pos) + "_in.e";
  }

  // If we're writing an Exodus file, write the Mesh using its logical
  // element dimension rather than the spatial dimension, unless it's
  // a 1D Mesh.  One reason to prefer this approach is that sidesets
  // are displayed incorrectly for 2D triangular elements in both
  // Paraview and Cubit if num_dim==3 in the Exodus file. We do the
  // same thing in MOOSE's Exodus Output object, so we are mimicking
  // that behavior here.
  if (mesh_file_name.find(".e") + 2 == mesh_file_name.size())
  {
    ExodusII_IO exio(mesh->getMesh());
    if (mesh->getMesh().mesh_dimension() != 1)
      exio.use_mesh_dimension_instead_of_spatial_dimension(true);

    exio.write(mesh_file_name);
  }
  else
  {
    // Just write the file using the name requested by the user.
    mesh->getMesh().write(mesh_file_name);
  }
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

void
MooseApp::setErrorOverridden()
{
  _error_overridden = true;
}

bool &
MooseApp::legacyUoAuxComputationDefault()
{
  return _legacy_uo_aux_computation_default;
}

bool &
MooseApp::legacyUoInitializationDefault()
{
  return _legacy_uo_initialization_default;
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
  _output_warehouse->meshChanged();

  if (_executioner.get() != NULL)
    _executioner->parentOutputPositionChanged();
}

std::string
MooseApp::getFileName(bool stripLeadingPath) const
{
  return _parser.getFileName(stripLeadingPath);
}

OutputWarehouse &
MooseApp::getOutputWarehouse()
{
  if (_alternate_output_warehouse == NULL)
    return *_output_warehouse;
  else
    return *_alternate_output_warehouse;
}
