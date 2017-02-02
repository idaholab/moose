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
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "JSONFormatter.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "CommandLine.h"
#include "InfixIterator.h"
#include "MultiApp.h"
#include "MeshModifier.h"
#include "DependencyResolver.h"
#include "MooseUtils.h"
#include "MooseObjectAction.h"
#include "InputParameterWarehouse.h"
#include "SystemInfo.h"
#include "RestartableDataIO.h"
#include "MooseMesh.h"
#include "FileOutput.h"
#include "ConsoleUtils.h"

// Regular expression includes
#include "pcrecpp.h"

// libMesh includes
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"

// System include for dynamic library methods
#include <dlfcn.h>
#include <sys/utsname.h> // utsname

// C++ includes
#include <numeric> // std::accumulate

#define QUOTE(macro) stringifyName(macro)

template<>
InputParameters validParams<MooseApp>()
{
  InputParameters params;

  params.addCommandLineParam<std::string>("input_file", "-i <input_file>", "Specify an input file");
  params.addCommandLineParam<std::string>("mesh_only", "--mesh-only", "Setup and Output the input mesh only.");

  params.addCommandLineParam<bool>("show_input", "--show-input", false, "Shows the parsed input file before running the simulation.");
  params.addCommandLineParam<bool>("show_outputs", "--show-outputs", false, "Shows the output execution time information.");
  params.addCommandLineParam<bool>("show_controls", "--show-controls", false, "Shows the Control logic available and executed.");

  params.addCommandLineParam<bool>("no_color", "--no-color", false, "Disable coloring of all Console outputs.");

  params.addCommandLineParam<bool>("help", "-h --help", false, "Displays CLI usage statement.");
  params.addCommandLineParam<bool>("minimal", "--minimal", false, "Ignore input file and build a minimal application with Transient executioner.");

  params.addCommandLineParam<std::string>("dump", "--dump [search_string]", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<std::string>("yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<std::string>("json", "--json", "Dumps input file syntax in JSON format.");
  params.addCommandLineParam<bool>("syntax", "--syntax", false, "Dumps the associated Action syntax paths ONLY");
  params.addCommandLineParam<bool>("check_input", "--check-input", false, "Check the input file (i.e. requires -i <filename>) and quit.");
  params.addCommandLineParam<bool>("list_constructed_objects", "--list-constructed-objects", false, "List all moose object type names constructed by the master app factory.");

  params.addCommandLineParam<unsigned int>("n_threads", "--n-threads=<n>", 1, "Runs the specified number of threads per process");

  params.addCommandLineParam<bool>("warn_unused", "-w --warn-unused", false, "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused", "-e --error-unused", false, "Error when encountering unused input file options");
  params.addCommandLineParam<bool>("error_override", "-o --error-override", false, "Error when encountering overridden or parameters supplied multiple times");
  params.addCommandLineParam<bool>("error_deprecated", "--error-deprecated", false, "Turn deprecated code messages into Errors");
  params.addCommandLineParam<bool>("allow_deprecated", "--allow-deprecated", false, "Can be used in conjunction with --error to turn off deprecated errors");

  params.addCommandLineParam<bool>("distributed_mesh", "--distributed-mesh", false, "The libMesh Mesh underlying MooseMesh should always be a DistributedMesh");

  params.addCommandLineParam<unsigned int>("refinements", "-r <n>", 0, "Specify additional initial uniform refinements for automatic scaling");

  params.addCommandLineParam<std::string>("recover", "--recover [file_base]", "Continue the calculation.  If file_base is omitted then the most recent recovery file will be utilized");

  params.addCommandLineParam<bool>("half_transient", "--half-transient", false, "When true the simulation will only run half of its specified transient (ie half the timesteps).  This is useful for testing recovery and restart");

  // No default on these two options, they must not both be valid
  params.addCommandLineParam<bool>("trap_fpe", "--trap-fpe", "Enable Floating Point Exception handling in critical sections of code.  This is enabled automatically in DEBUG mode");
  params.addCommandLineParam<bool>("no_trap_fpe", "--no-trap-fpe", "Disable Floating Point Exception handling in critical sections of code when using DEBUG mode.");

  params.addCommandLineParam<bool>("error", "--error", false, "Turn all warnings into errors");

  params.addCommandLineParam<bool>("timing", "-t --timing", false, "Enable all performance logging for timing purposes. This will disable all screen output of performance logs for all Console objects.");

  // Legacy Flags
  params.addParam<bool>("use_legacy_uo_aux_computation", false, "Set to true to have MOOSE recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");
  params.addParam<bool>("use_legacy_uo_initialization", false, "Set to true to have MOOSE compute all UserObjects and Postprocessors during the initial setup phase of the problem recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");

  // Options ignored by MOOSE but picked up by libMesh, these are here so that they are displayed in the application help
  params.addCommandLineParam<bool>("keep_cout", "--keep-cout", false, "Keep standard output from all processors when running in parallel");
  params.addCommandLineParam<bool>("redirect_stdout", "--redirect-stdout", false, "Keep standard output from all processors when running in parallel");


  params.addPrivateParam<std::string>("_app_name"); // the name passed to AppFactory::create
  params.addPrivateParam<std::string>("_type");
  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char**>("_argv");
  params.addPrivateParam<std::shared_ptr<CommandLine>>("_command_line");
  params.addPrivateParam<std::shared_ptr<Parallel::Communicator>>("_comm");

  return params;
}

MooseApp::MooseApp(InputParameters parameters) :
    ConsoleStreamInterface(*this),
    ParallelObject(*parameters.get<std::shared_ptr<Parallel::Communicator>>("_comm")), // Can't call getParam() before pars is set
    _name(parameters.get<std::string>("_app_name")),
    _pars(parameters),
    _type(getParam<std::string>("_type")),
    _comm(getParam<std::shared_ptr<Parallel::Communicator>>("_comm")),
    _output_position_set(false),
    _start_time_set(false),
    _start_time(0.0),
    _global_time_offset(0.0),
    _output_warehouse(*this),
    _input_parameter_warehouse(new InputParameterWarehouse()),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _parser(*this, _action_warehouse),
    _use_nonlinear(true),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _initial_from_file(false),
    _distributed_mesh_on_command_line(false),
    _recover(false),
    _restart(false),
    _half_transient(false),
    _legacy_uo_aux_computation_default(getParam<bool>("use_legacy_uo_aux_computation")),
    _legacy_uo_initialization_default(getParam<bool>("use_legacy_uo_initialization")),
    _check_input(getParam<bool>("check_input")),
    _restartable_data(libMesh::n_threads()),
    _multiapp_level(0),
    _use_name_prefix(false)
{
  if (isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char**>("_argv");

    _sys_info = std::make_shared<SystemInfo>(argc, argv);
  }
  if (isParamValid("_command_line"))
    _command_line = getParam<std::shared_ptr<CommandLine>>("_command_line");
  else
    mooseError2("Valid CommandLine object required");

  if (getParam<bool>("error_deprecated") && getParam<bool>("allow_deprecated"))
    mooseError2("Both error deprecated and allowed deprecated were set.");
}

MooseApp::~MooseApp()
{
  _action_warehouse.clear();
  _executioner.reset();

  delete _input_parameter_warehouse;

#ifdef LIBMESH_HAVE_DLOPEN
  // Close any open dynamic libraries
  for (const auto & it : _lib_handles)
    dlclose(it.second);
#endif
}

void
MooseApp::setupOptions()
{
  // Print the header, this is as early as possible
  std::string hdr(header() + "\n");
  if (useNamePrefix())
    MooseUtils::indentMessage(_name, hdr);
  Moose::out << hdr << std::flush;

  if (getParam<bool>("error_unused"))
    setCheckUnusedFlag(true);
  else if (getParam<bool>("warn_unused"))
    setCheckUnusedFlag(false);

  if (getParam<bool>("error_override"))
    setErrorOverridden();

  _distributed_mesh_on_command_line = getParam<bool>("distributed_mesh");

  _half_transient = getParam<bool>("half_transient");
  _pars.set<bool>("timing") = getParam<bool>("timing");

  if (isParamValid("trap_fpe") && isParamValid("no_trap_fpe"))
    mooseError2("Cannot use both \"--trap-fpe\" and \"--no-trap-fpe\" flags.");
  if (isParamValid("trap_fpe"))
    Moose::_trap_fpe = true;
  else if (isParamValid("no_trap_fpe"))
    Moose::_trap_fpe = false;

  // Turn all warnings in MOOSE to errors (almost see next logic block)
  Moose::_warnings_are_errors = getParam<bool>("error");

  /**
   * Deprecated messages can be toggled to errors independently from everything else.
   * Normally they are toggled with the --error flag but that behavior can
   * be modified with the --allow-warnings.
   */
  if (getParam<bool>("error_deprecated") ||
      (Moose::_warnings_are_errors && !getParam<bool>("allow_deprecated")))
    Moose::_deprecated_is_error = true;
  else
    Moose::_deprecated_is_error = false;

  // Toggle the color console off
  Moose::_color_console = !getParam<bool>("no_color");

  // If there's no threading model active, but the user asked for
  // --n-threads > 1 on the command line, throw a mooseError2.  This is
  // intended to prevent situations where the user has potentially
  // built MOOSE incorrectly (neither TBB nor pthreads found) and is
  // asking for multiple threads, not knowing that there will never be
  // any threads launched.
#if !LIBMESH_USING_THREADS
  if (libMesh::command_line_value ("--n-threads", 1) > 1)
    mooseError2("You specified --n-threads > 1, but there is no threading model active!");
#endif

  // Build a minimal running application, ignoring the input file.
  if (getParam<bool>("minimal"))
    createMinimalApp();

  else if (getParam<bool>("help"))
  {
    Moose::perf_log.disable_logging();

    _command_line->printUsage();
    _ready_to_exit = true;
  }
  else if (isParamValid("dump"))
  {
    Moose::perf_log.disable_logging();

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
    Moose::perf_log.disable_logging();

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
  else if (isParamValid("json"))
  {
    Moose::perf_log.disable_logging();

    _parser.initSyntaxFormatter(Parser::JSON, true);

    // Get command line argument following --json on command line
    std::string json_following_arg = getParam<std::string>("json");

    // If the argument following --json is non-existent or begins with
    // a dash, call buildFullTree() with an empty string, otherwise
    // pass the argument following --json.
    if (json_following_arg.empty() || (json_following_arg.find('-') == 0))
      _parser.buildFullTree("");
    else
      _parser.buildFullTree(json_following_arg);

    _ready_to_exit = true;
  }
  else if (getParam<bool>("syntax"))
  {
    Moose::perf_log.disable_logging();

    std::multimap<std::string, Syntax::ActionInfo> syntax = _syntax.getAssociatedActions();
    Moose::out << "**START SYNTAX DATA**\n";
    for (const auto & it : syntax)
      Moose::out << it.first << "\n";
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
    Moose::perf_log.disable_logging();

    if (_check_input)
      mooseError2("You specified --check-input, but did not provide an input file. Add -i <inputfile> to your command line.");

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

  if (getParam<bool>("list_constructed_objects"))
  {
    // TODO: ask multiapps for their constructed objects
    std::vector<std::string> obj_list = _factory.getConstructedObjects();
    Moose::out << "**START OBJECT DATA**\n";
    for (const auto & name : obj_list)
      Moose::out << name << "\n";
    Moose::out << "**END OBJECT DATA**\n" << std::endl;
    _ready_to_exit = true;
    return;
  }


  bool error_unused = getParam<bool>("error_unused") || _enable_unused_check == ERROR_UNUSED;
  bool warn_unused = getParam<bool>("warn_unused") || _enable_unused_check == WARN_UNUSED;

  if (error_unused || warn_unused)
  {
    std::shared_ptr<FEProblemBase> fe_problem = _action_warehouse.problemBase();
    if (fe_problem.get() && name() == "main" && !getParam<bool>("minimal"))
    {
      // Check the CLI parameters
      std::vector<std::string> all_vars = _command_line->getPot()->get_variable_names();
      _parser.checkUnidentifiedParams(all_vars, error_unused, false, fe_problem);

      // Check the input file parameters
      all_vars = _parser.getPotHandle()->get_variable_names();
      _parser.checkUnidentifiedParams(all_vars, error_unused, true, fe_problem);
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
    mooseError2("No executioner was specified (go fix your input file)");
}

bool
MooseApp::isRecovering() const
{
  return _recover;
}

bool
MooseApp::isRestarting() const
{
  return _restart;
}

bool
MooseApp::hasRecoverFileBase()
{
  return !_recover_base.empty();
}

void
MooseApp::meshOnly(std::string mesh_file_name)
{
  /**
   * These actions should be the minimum set necessary to generate and output
   * a Mesh.
   */
  _action_warehouse.executeActionsWithAction("meta_action");
  _action_warehouse.executeActionsWithAction("set_global_params");
  _action_warehouse.executeActionsWithAction("setup_mesh");
  _action_warehouse.executeActionsWithAction("add_partitioner");
  _action_warehouse.executeActionsWithAction("init_mesh");
  _action_warehouse.executeActionsWithAction("prepare_mesh");
  _action_warehouse.executeActionsWithAction("add_mesh_modifier");
  _action_warehouse.executeActionsWithAction("execute_mesh_modifiers");
  _action_warehouse.executeActionsWithAction("uniform_refine_mesh");
  _action_warehouse.executeActionsWithAction("setup_mesh_complete");

  std::shared_ptr<MooseMesh> & mesh = _action_warehouse.mesh();

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
MooseApp::registerRecoverableData(std::string name)
{
  _recoverable_data.insert(name);
}

std::shared_ptr<Backup>
MooseApp::backup()
{
  FEProblemBase & fe_problem = _executioner->feProblem();

  RestartableDataIO rdio(fe_problem);

  return rdio.createBackup();
}

void
MooseApp::restore(std::shared_ptr<Backup> backup, bool for_restart)
{
  // This means that a Backup is coming through to use for restart / recovery
  // We should just cache it for now
  if (!_executioner)
  {
    _cached_backup = backup;
    return;
  }

  FEProblemBase & fe_problem = _executioner->feProblem();

  RestartableDataIO rdio(fe_problem);

  rdio.restoreBackup(backup, for_restart);
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
  Moose::perf_log.push("Full Runtime", "Application");

  Moose::perf_log.push("Application Setup", "Setup");
  setupOptions();
  runInputFile();
  Moose::perf_log.pop("Application Setup", "Setup");

  executeExecutioner();
  Moose::perf_log.pop("Full Runtime", "Application");
}

void
MooseApp::setOutputPosition(Point p)
{
  _output_position_set = true;
  _output_position = p;
  _output_warehouse.meshChanged();

  if (_executioner.get() != NULL)
    _executioner->parentOutputPositionChanged();
}

std::list<std::string>
MooseApp::getCheckpointFiles()
{
  // Extract the CommonOutputAction
  const auto & common_actions = _action_warehouse.getActionListByName("common_output");
  mooseAssert(common_actions.size() == 1, "Should be only one common_output Action");

  const Action * common = *common_actions.begin();

  // Storage for the directory names
  std::list<std::string> checkpoint_dirs;

  // If file_base is set in CommonOutputAction, add this file to the list of potential checkpoint files
  if (common->isParamValid("file_base"))
    checkpoint_dirs.push_back(common->getParam<std::string>("file_base") + "_cp");
  // Case for normal application or master in a Multiapp setting
  else if (getOutputFileBase().empty())
    checkpoint_dirs.push_back(FileOutput::getOutputFileBase(*this, "_out_cp"));
  // Case for a sub app in a Multiapp setting
  else
    checkpoint_dirs.push_back(getOutputFileBase() + "_cp");

  // Add the directories from any existing checkpoint objects
  const auto & actions = _action_warehouse.getActionListByName("add_output");
  for (const auto & action : actions)
  {
    // Get the parameters from the MooseObjectAction
    MooseObjectAction * moose_object_action = static_cast<MooseObjectAction *>(action);
    const InputParameters & params = moose_object_action->getObjectParams();

    // Loop through the actions and add the necessary directories to the list to check
    if (moose_object_action->getParam<std::string>("type") == "Checkpoint")
    {
      if (params.isParamValid("file_base"))
        checkpoint_dirs.push_back(common->getParam<std::string>("file_base") + "_cp");
      else
      {
        std::ostringstream oss;
        oss << "_" << action->name() << "_cp";
        checkpoint_dirs.push_back(FileOutput::getOutputFileBase(*this, oss.str()));
      }
    }
  }

  return MooseUtils::getFilesInDirs(checkpoint_dirs);
}

void
MooseApp::setStartTime(const Real time)
{
  _start_time_set = true;
  _start_time = time;
}

std::string
MooseApp::getFileName(bool stripLeadingPath) const
{
  return _parser.getFileName(stripLeadingPath);
}

OutputWarehouse &
MooseApp::getOutputWarehouse()
{
  return _output_warehouse;
}

std::string
MooseApp::appNameToLibName(const std::string & app_name) const
{
  std::string library_name(app_name);

  // Strip off the App part (should always be the last 3 letters of the name)
  size_t pos = library_name.find("App");
  if (pos != library_name.length() - 3)
    mooseError2("Invalid application name: ", library_name);
  library_name.erase(pos);

  // Now get rid of the camel case, prepend lib, and append the method and suffix
  return std::string("lib") + MooseUtils::camelCaseToUnderscore(library_name) + '-' + QUOTE(METHOD) + ".la";
}

std::string
MooseApp::libNameToAppName(const std::string & library_name) const
{
  std::string app_name(library_name);

  // Strip off the leading "lib" and trailing ".la"
  if (pcrecpp::RE("lib(.+?)(?:-\\w+)?\\.la").Replace("\\1", &app_name) == 0)
    mooseError2("Invalid library name: ", app_name);

  return MooseUtils::underscoreToCamelCase(app_name, true);
}


void
MooseApp::registerRestartableData(std::string name, RestartableDataValue * data, THREAD_ID tid)
{
  std::map<std::string, RestartableDataValue *> & restartable_data = _restartable_data[tid];

  if (restartable_data.find(name) != restartable_data.end())
    mooseError2("Attempted to declare restartable twice with the same name: ", name);

  restartable_data[name] = data;
}

void
MooseApp::dynamicAppRegistration(const std::string & app_name, std::string library_path)
{
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = APPLICATION;
  params.set<std::string>("registration_method") = app_name + "__registerApps";
  params.set<std::string>("library_path") = library_path;

  dynamicRegistration(params);

  // At this point the application should be registered so check it
  if (!AppFactory::instance().isRegistered(app_name))
  {
    std::ostringstream oss;
    std::set<std::string> paths = getLoadedLibraryPaths();

    oss << "Unable to locate library for \"" << app_name << "\".\nWe attempted to locate the library \"" << appNameToLibName(app_name)
        << "\" in the following paths:\n\t";
    std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
    oss << "\n\nMake sure you have compiled the library and either set the \"library_path\" variable "
        << "in your input file or exported \"MOOSE_LIBRARY_PATH\".\n"
        << "Compiled in debug mode to see the list of libraries checked for dynamic loading methods.";
    mooseError2(oss.str());
  }
}

void
MooseApp::dynamicObjectRegistration(const std::string & app_name, Factory * factory, std::string library_path)
{
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = OBJECT;
  params.set<std::string>("registration_method") = app_name + "__registerObjects";
  params.set<std::string>("library_path") = library_path;

  params.set<Factory *>("factory") = factory;

  dynamicRegistration(params);
}

void
MooseApp::dynamicSyntaxAssociation(const std::string & app_name, Syntax * syntax, ActionFactory * action_factory, std::string library_path)
{
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = SYNTAX;
  params.set<std::string>("registration_method") = app_name + "__associateSyntax";
  params.set<std::string>("library_path") = library_path;

  params.set<Syntax *>("syntax") = syntax;
  params.set<ActionFactory *>("action_factory") = action_factory;

  dynamicRegistration(params);
}

void
MooseApp::dynamicRegistration(const Parameters & params)
{
  // first convert the app name to a library name
  std::string library_name = appNameToLibName(params.get<std::string>("app_name"));

  // Create a vector of paths that we can search inside for libraries
  std::vector<std::string> paths;

  std::string library_path = params.get<std::string>("library_path");

  if (library_path != "")
    MooseUtils::tokenize(library_path, paths, 1, ":");

  char * moose_lib_path_env = std::getenv("MOOSE_LIBRARY_PATH");
  if (moose_lib_path_env)
  {
    std::string moose_lib_path(moose_lib_path_env);
    std::vector<std::string> tmp_paths;

    MooseUtils::tokenize(moose_lib_path, tmp_paths, 1, ":");

    // merge the two vectors together (all possible search paths)
    paths.insert(paths.end(), tmp_paths.begin(), tmp_paths.end());
  }

  // Attempt to dynamically load the library
  for (const auto & path : paths)
    if (MooseUtils::checkFileReadable(path + '/' + library_name, false, false))
      loadLibraryAndDependencies(path + '/' + library_name, params);
    else
      mooseWarning2("Unable to open library file \"", path + '/' + library_name, "\". Double check for spelling errors.");
}

void
MooseApp::loadLibraryAndDependencies(const std::string & library_filename, const Parameters & params)
{
  std::string line;
  std::string dl_lib_filename;

  // This RE looks for absolute path libtool filenames (i.e. begins with a slash and ends with a .la)
  pcrecpp::RE re_deps("(/\\S*\\.la)");

  std::ifstream handle(library_filename.c_str());
  if (handle.is_open())
  {
    while (std::getline(handle, line))
    {
      // Look for the system dependent dynamic library filename to open
      if (line.find("dlname=") != std::string::npos)
        // Magic numbers are computed from length of this string "dlname=' and line minus that string plus quotes"
        dl_lib_filename = line.substr(8, line.size()-9);

      if (line.find("dependency_libs=") != std::string::npos)
      {
        pcrecpp::StringPiece input(line);
        pcrecpp::StringPiece depend_library;
        while (re_deps.FindAndConsume(&input, &depend_library))
          // Recurse here to load dependent libraries in depth-first order
          loadLibraryAndDependencies(depend_library.as_string(), params);

        // There's only one line in the .la file containing the dependency libs so break after finding it
        break;
      }
    }
    handle.close();
  }

  std::string registration_method_name = params.get<std::string>("registration_method");
  // Time to load the library, First see if we've already loaded this particular dynamic library
  if (_lib_handles.find(std::make_pair(library_filename, registration_method_name)) == _lib_handles.end() && // make sure we haven't already loaded this library
      dl_lib_filename != "")                                                                                 // AND make sure we have a library name (we won't for static linkage)
  {
    std::pair<std::string, std::string> lib_name_parts = MooseUtils::splitFileName(library_filename);

    // Assemble the actual filename using the base path of the *.la file and the dl_lib_filename
    std::string dl_lib_full_path = lib_name_parts.first + '/' + dl_lib_filename;

#ifdef LIBMESH_HAVE_DLOPEN
    void * handle = dlopen(dl_lib_full_path.c_str(), RTLD_LAZY);
#else
    void * handle = NULL;
#endif

    if (!handle)
      mooseError2("Cannot open library: ", dl_lib_full_path.c_str(), "\n");

    // get the pointer to the method in the library.  The dlsym()
    // function returns a null pointer if the symbol cannot be found,
    // we also explicitly set the pointer to NULL if dlsym is not
    // available.
#ifdef LIBMESH_HAVE_DLOPEN
    void * registration_method = dlsym(handle, registration_method_name.c_str());
#else
    void * registration_method = NULL;
#endif

    if (!registration_method)
    {
      // We found a dynamic library that doesn't have a dynamic
      // registration method in it. This shouldn't be an error, so
      // we'll just move on.
#ifdef DEBUG
      mooseWarning2("Unable to find extern \"C\" method \"", registration_method_name,
                    "\" in library: ", dl_lib_full_path, ".\n",
                    "This doesn't necessarily indicate an error condition unless you believe that the method should exist in that library.\n");
#endif

#ifdef LIBMESH_HAVE_DLOPEN
      dlclose(handle);
#endif
    }
    else // registration_method is valid!
    {
      // TODO: Look into cleaning this up
      switch (params.get<RegistrationType>("reg_type"))
      {
      case APPLICATION:
      {
        typedef void (*register_app_t)();
        register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );
        (*reg_ptr)();
        break;
      }
      case OBJECT:
      {
        typedef void (*register_app_t)(Factory *);
        register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );
        (*reg_ptr)(params.get<Factory *>("factory"));
        break;
      }
      case SYNTAX:
      {
        typedef void (*register_app_t)(Syntax *, ActionFactory *);
        register_app_t *reg_ptr = reinterpret_cast<register_app_t *>( &registration_method );
        (*reg_ptr)(params.get<Syntax *>("syntax"), params.get<ActionFactory *>("action_factory"));
        break;
      }
      default:
        mooseError2("Unhandled RegistrationType");
      }

      // Store the handle so we can close it later
      _lib_handles.insert(std::make_pair(std::make_pair(library_filename, registration_method_name), handle));
    }
  }
}

std::set<std::string>
MooseApp::getLoadedLibraryPaths() const
{
  // Return the paths but not the open file handles
  std::set<std::string> paths;
  for (const auto & it : _lib_handles)
    paths.insert(it.first.first);

  return paths;
}

InputParameterWarehouse &
MooseApp::getInputParameterWarehouse()
{
  return *_input_parameter_warehouse;
}

std::string
MooseApp::header() const
{
  return std::string("");
}

void
MooseApp::addMeshModifier(const std::string & modifier_name, const std::string & name, InputParameters parameters)
{
  std::shared_ptr<MeshModifier> mesh_modifier = _factory.create<MeshModifier>(modifier_name, name, parameters);

  _mesh_modifiers.insert(std::make_pair(MooseUtils::shortName(name), mesh_modifier));
}

const MeshModifier &
MooseApp::getMeshModifier(const std::string & name) const
{
  return *_mesh_modifiers.find(MooseUtils::shortName(name))->second.get();
}

void
MooseApp::executeMeshModifiers()
{
  DependencyResolver<std::shared_ptr<MeshModifier>> resolver;

  // Add all of the dependencies into the resolver and sort them
  for (const auto & it : _mesh_modifiers)
  {
    // Make sure an item with no dependencies comes out too!
    resolver.addItem(it.second);

    std::vector<std::string> & modifiers = it.second->getDependencies();
    for (const auto & depend_name : modifiers)
    {
      auto depend_it = _mesh_modifiers.find(depend_name);

      if (depend_it == _mesh_modifiers.end())
        mooseError2("The MeshModifier \"", depend_name, "\" was not created, did you make a spelling mistake or forget to include it in your input file?");

      resolver.insertDependency(it.second, depend_it->second);
    }
  }

  const auto & ordered_modifiers = resolver.getSortedValues();

  if (ordered_modifiers.size())
  {
    MooseMesh * mesh = _action_warehouse.mesh().get();
    MooseMesh * displaced_mesh = _action_warehouse.displacedMesh().get();

    // Run the MeshModifiers in the proper order
    for (const auto & modifier : ordered_modifiers)
      modifier->modifyMesh(mesh, displaced_mesh);

    /**
     * Set preparation flag after modifers are run. The final preparation
     * will be handled by the SetupMeshComplete Action.
     */
    mesh->prepared(false);
    if (displaced_mesh)
      displaced_mesh->prepared(false);
  }
}

void
MooseApp::clearMeshModifiers()
{
  _mesh_modifiers.clear();
}

void
MooseApp::setRestart(const bool & value)
{
  _restart = value;

  std::shared_ptr<FEProblemBase> fe_problem = _action_warehouse.problemBase();
}

void
MooseApp::setRecover(const bool & value)
{
  _recover = value;
}


void
MooseApp::restoreCachedBackup()
{
  if (!_cached_backup.get())
    mooseError2("No cached Backup to restore!");

  restore(_cached_backup, isRestarting());

  // Release our hold on this Backup
  _cached_backup.reset();
}


void
MooseApp::createMinimalApp()
{
  // SetupMeshAction (setup_mesh)
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");
    action_params.set<std::string>("type") = "GeneratedMesh";
    action_params.set<std::string>("task") = "setup_mesh";

    // Create The Action
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(_action_factory.create("SetupMeshAction", "Mesh", action_params));

    // Set the object parameters
    InputParameters & params = action->getObjectParams();
    params.set<MooseEnum>("dim") = "1";
    params.set<unsigned int>("nx") = 1;

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  // SetupMeshAction (init_mesh)
  {
    // Action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");
    action_params.set<std::string>("type") = "GeneratedMesh";
    action_params.set<std::string>("task") = "init_mesh";

    // Build the action
    std::shared_ptr<Action> action = _action_factory.create("SetupMeshAction", "Mesh", action_params);
    _action_warehouse.addActionBlock(action);
  }

  // Executioner
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CreateExecutionerAction");
    action_params.set<std::string>("type") = "Transient";

    // Create the action
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(_action_factory.create("CreateExecutionerAction", "Executioner", action_params));

    // Set the object parameters
    InputParameters & params = action->getObjectParams();
    params.set<unsigned int>("num_steps") = 1;
    params.set<Real>("dt") = 1;

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  // Problem
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CreateProblemAction");
    action_params.set<std::string>("type") = "FEProblem";

    // Create the action
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(_action_factory.create("CreateProblemAction", "Problem", action_params));

    // Set the object parameters
    InputParameters & params = action->getObjectParams();
    params.set<bool>("solve") = false;

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  // Outputs
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CommonOutputAction");
    action_params.set<bool>("console") = false;

    // Create action
    std::shared_ptr<Action> action = _action_factory.create("CommonOutputAction", "Outputs", action_params);

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  _action_warehouse.build();
}
