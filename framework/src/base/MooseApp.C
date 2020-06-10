//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef HAVE_GPERFTOOLS
#include "gperftools/profiler.h"
#endif

// MOOSE includes
#include "MooseRevision.h"
#include "AppFactory.h"
#include "DisplacedProblem.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "CommandLine.h"
#include "InfixIterator.h"
#include "MultiApp.h"
#include "MeshModifier.h"
#include "MeshGenerator.h"
#include "DependencyResolver.h"
#include "MooseUtils.h"
#include "MooseObjectAction.h"
#include "InputParameterWarehouse.h"
#include "SystemInfo.h"
#include "RestartableDataIO.h"
#include "MooseMesh.h"
#include "FileOutput.h"
#include "ConsoleUtils.h"
#include "JsonSyntaxTree.h"
#include "JsonInputFileFormatter.h"
#include "SONDefinitionFormatter.h"
#include "RelationshipManager.h"
#include "Registry.h"
#include "SerializerGuard.h"
#include "PerfGraphInterface.h" // For TIME_SECTIOn
#include "Attributes.h"
#include "MooseApp.h"

// Regular expression includes
#include "pcrecpp.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/checkpoint_io.h"
#include "libmesh/mesh_base.h"

// System include for dynamic library methods
#ifdef LIBMESH_HAVE_DLOPEN
#include <dlfcn.h>
#include <sys/utsname.h> // utsname
#endif

// C++ includes
#include <numeric> // std::accumulate
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib> // for system()
#include <chrono>
#include <thread>

#define QUOTE(macro) stringifyName(macro)

defineLegacyParams(MooseApp);

InputParameters
MooseApp::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addCommandLineParam<bool>(
      "display_version", "-v --version", false, "Print application version");
  params.addCommandLineParam<std::string>("input_file", "-i <input_file>", "Specify an input file");
  params.addCommandLineParam<std::string>(
      "mesh_only",
      "--mesh-only [mesh_file_name]",
      "Setup and Output the input mesh only (Default: \"<input_file_name>_in.e\")");

  params.addCommandLineParam<bool>("show_input",
                                   "--show-input",
                                   false,
                                   "Shows the parsed input file before running the simulation.");
  params.addCommandLineParam<bool>(
      "show_outputs", "--show-outputs", false, "Shows the output execution time information.");
  params.addCommandLineParam<bool>(
      "show_controls", "--show-controls", false, "Shows the Control logic available and executed.");

  params.addCommandLineParam<bool>(
      "no_color", "--no-color", false, "Disable coloring of all Console outputs.");
  params.addCommandLineParam<std::string>("color",
                                          "--color [auto,on,off]",
                                          "default-on",
                                          "Whether to use color in console output (default 'on').");

  params.addCommandLineParam<bool>("help", "-h --help", false, "Displays CLI usage statement.");
  params.addCommandLineParam<bool>(
      "minimal",
      "--minimal",
      false,
      "Ignore input file and build a minimal application with Transient executioner.");

  params.addCommandLineParam<std::string>(
      "definition", "--definition", "Shows a SON style input definition dump for input validation");
  params.addCommandLineParam<std::string>(
      "dump", "--dump [search_string]", "Shows a dump of available input file syntax.");
  params.addCommandLineParam<bool>(
      "registry", "--registry", "Lists all known objects and actions.");
  params.addCommandLineParam<bool>(
      "registry_hit", "--registry-hit", "Lists all known objects and actions in hit format.");

  params.addCommandLineParam<bool>(
      "apptype", "--type", false, "Return the name of the application object.");
  params.addCommandLineParam<std::string>(
      "yaml", "--yaml", "Dumps input file syntax in YAML format.");
  params.addCommandLineParam<std::string>(
      "json", "--json", "Dumps input file syntax in JSON format.");
  params.addCommandLineParam<bool>(
      "syntax", "--syntax", false, "Dumps the associated Action syntax paths ONLY");
  params.addCommandLineParam<bool>("check_input",
                                   "--check-input",
                                   false,
                                   "Check the input file (i.e. requires -i <filename>) and quit.");
  params.addCommandLineParam<bool>(
      "list_constructed_objects",
      "--list-constructed-objects",
      false,
      "List all moose object type names constructed by the master app factory.");

  params.addCommandLineParam<unsigned int>(
      "n_threads", "--n-threads=<n>", 1, "Runs the specified number of threads per process");

  params.addCommandLineParam<bool>(
      "warn_unused", "-w --warn-unused", false, "Warn about unused input file options");
  params.addCommandLineParam<bool>("error_unused",
                                   "-e --error-unused",
                                   false,
                                   "Error when encountering unused input file options");
  params.addCommandLineParam<bool>(
      "error_override",
      "-o --error-override",
      false,
      "Error when encountering overridden or parameters supplied multiple times");
  params.addCommandLineParam<bool>(
      "error_deprecated", "--error-deprecated", false, "Turn deprecated code messages into Errors");

  params.addCommandLineParam<bool>(
      "distributed_mesh",
      "--distributed-mesh",
      false,
      "The libMesh Mesh underlying MooseMesh should always be a DistributedMesh");

  params.addCommandLineParam<std::string>(
      "split_mesh",
      "--split-mesh [splits]",
      "comma-separated list of numbers of chunks to split the mesh into");

  params.addCommandLineParam<std::string>("split_file",
                                          "--split-file [filename]",
                                          "",
                                          "optional name of split mesh file(s) to write/read");

  params.addCommandLineParam<bool>(
      "use_split", "--use-split", false, "use split distributed mesh files");

  params.addCommandLineParam<unsigned int>(
      "refinements",
      "-r <n>",
      0,
      "Specify additional initial uniform refinements for automatic scaling");

  params.addCommandLineParam<std::string>("recover",
                                          "--recover [file_base]",
                                          "Continue the calculation.  If file_base is omitted then "
                                          "the most recent recovery file will be utilized");

  params.addCommandLineParam<std::string>("recoversuffix",
                                          "--recoversuffix [suffix]",
                                          "Use a different file extension, other than cpr, "
                                          "for a recovery file");

  params.addCommandLineParam<bool>("half_transient",
                                   "--half-transient",
                                   false,
                                   "When true the simulation will only run half of "
                                   "its specified transient (ie half the "
                                   "timesteps).  This is useful for testing "
                                   "recovery and restart");

  // No default on these two options, they must not both be valid
  params.addCommandLineParam<bool>(
      "trap_fpe",
      "--trap-fpe",
      "Enable Floating Point Exception handling in critical sections of "
      "code.  This is enabled automatically in DEBUG mode");
  params.addCommandLineParam<bool>("no_trap_fpe",
                                   "--no-trap-fpe",
                                   "Disable Floating Point Exception handling in critical "
                                   "sections of code when using DEBUG mode.");

  params.addCommandLineParam<bool>("error", "--error", false, "Turn all warnings into errors");

  params.addCommandLineParam<bool>(
      "timing",
      "-t --timing",
      false,
      "Enable all performance logging for timing purposes. This will disable all "
      "screen output of performance logs for all Console objects.");
  params.addCommandLineParam<bool>("no_timing",
                                   "--no-timing",
                                   false,
                                   "Disabled performance logging. Overrides -t or --timing "
                                   "if passed in conjunction with this flag");

  params.addCommandLineParam<bool>(
      "allow_test_objects", "--allow-test-objects", false, "Register test objects and syntax.");

  // Options ignored by MOOSE but picked up by libMesh, these are here so that they are displayed in
  // the application help
  params.addCommandLineParam<bool>(
      "keep_cout",
      "--keep-cout",
      false,
      "Keep standard output from all processors when running in parallel");
  params.addCommandLineParam<bool>(
      "redirect_stdout",
      "--redirect-stdout",
      false,
      "Keep standard output from all processors when running in parallel");

  // Options for debugging
  params.addCommandLineParam<std::string>("start_in_debugger",
                                          "--start-in-debugger <debugger>",
                                          "Start the application and attach a debugger.  This will "
                                          "launch xterm windows using the command you specify for "
                                          "'debugger'");

  params.addCommandLineParam<unsigned int>("stop_for_debugger",
                                           "--stop-for-debugger [seconds]",
                                           30,
                                           "Pauses the application during startup for the "
                                           "specified time to allow for connection of debuggers.");
  params.addParam<bool>(
      "automatic_automatic_scaling", false, "Whether to turn on automatic scaling by default.");

  params.addPrivateParam<std::string>("_app_name"); // the name passed to AppFactory::create
  params.addPrivateParam<std::string>("_type");
  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char **>("_argv");
  params.addPrivateParam<std::shared_ptr<CommandLine>>("_command_line");
  params.addPrivateParam<std::shared_ptr<Parallel::Communicator>>("_comm");
  params.addPrivateParam<unsigned int>("_multiapp_level");
  params.addPrivateParam<unsigned int>("_multiapp_number");
  params.addPrivateParam<const MooseMesh *>("_master_mesh");
  params.addPrivateParam<const MooseMesh *>("_master_displaced_mesh");

  params.addParam<bool>(
      "use_legacy_dirichlet_bc",
      true,
      "Set false to have MOOSE utilize the new and preferred method of setting preset = true as "
      "default for all DirichletBC and derived objects, which is ideal for the majority of solves "
      "utilizing this boundary condition type.\nThe old behavior (which is utilized if this is set "
      "to true) is to set preset = false as the default for DirichletBC and derived objects.");
  params.addParam<bool>(
      "use_legacy_material_output",
      true,
      "Set false to allow material properties to be output on INITIAL, not just TIMESTEP_END.");

  return params;
}

MooseApp::MooseApp(InputParameters parameters)
  : ConsoleStreamInterface(*this),
    ParallelObject(*parameters.get<std::shared_ptr<Parallel::Communicator>>(
        "_comm")), // Can't call getParam() before pars is set
    _name(parameters.get<std::string>("_app_name")),
    _pars(parameters),
    _type(getParam<std::string>("_type")),
    _comm(getParam<std::shared_ptr<Parallel::Communicator>>("_comm")),
    _perf_graph(type() + " (" + name() + ')'),
    _rank_map(*_comm, _perf_graph),
    _file_base_set_by_user(false),
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
    _use_eigen_value(false),
    _enable_unused_check(WARN_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _initial_from_file(false),
    _distributed_mesh_on_command_line(false),
    _recover(false),
    _restart(false),
    _split_mesh(false),
    _use_split(parameters.get<bool>("use_split")),
#ifdef DEBUG
    _trap_fpe(true),
#else
    _trap_fpe(false),
#endif
    _restart_recover_suffix("cpr"),
    _half_transient(false),
    _check_input(getParam<bool>("check_input")),
    _restartable_data(libMesh::n_threads()),
    _multiapp_level(
        isParamValid("_multiapp_level") ? parameters.get<unsigned int>("_multiapp_level") : 0),
    _multiapp_number(
        isParamValid("_multiapp_number") ? parameters.get<unsigned int>("_multiapp_number") : 0),
    _master_mesh(isParamValid("_master_mesh") ? parameters.get<const MooseMesh *>("_master_mesh")
                                              : nullptr),
    _master_displaced_mesh(isParamValid("_master_displaced_mesh")
                               ? parameters.get<const MooseMesh *>("_master_displaced_mesh")
                               : nullptr),
    _setup_timer(_perf_graph.registerSection("MooseApp::setup", 2)),
    _setup_options_timer(_perf_graph.registerSection("MooseApp::setupOptions", 5)),
    _run_input_file_timer(_perf_graph.registerSection("MooseApp::runInputFile", 3)),
    _execute_timer(_perf_graph.registerSection("MooseApp::execute", 2)),
    _execute_executioner_timer(_perf_graph.registerSection("MooseApp::executeExecutioner", 3)),
    _restore_timer(_perf_graph.registerSection("MooseApp::restore", 2)),
    _run_timer(_perf_graph.registerSection("MooseApp::run", 3)),
    _execute_mesh_modifiers_timer(_perf_graph.registerSection("MooseApp::executeMeshModifiers", 1)),
    _execute_mesh_generators_timer(
        _perf_graph.registerSection("MooseApp::executeMeshGenerators", 1)),
    _restore_cached_backup_timer(_perf_graph.registerSection("MooseApp::restoreCachedBackup", 2)),
    _create_minimal_app_timer(_perf_graph.registerSection("MooseApp::createMinimalApp", 3)),
    _automatic_automatic_scaling(getParam<bool>("automatic_automatic_scaling")),
    _popped_final_mesh_generator(false)
{
#ifdef HAVE_GPERFTOOLS
  if (std::getenv("MOOSE_PROFILE_BASE"))
  {
    static std::string profile_file =
        std::getenv("MOOSE_PROFILE_BASE") + std::to_string(_comm->rank()) + ".prof";
    _profiling = true;
    ProfilerStart(profile_file.c_str());
  }
#endif

  Registry::addKnownLabel(_type);
  Moose::registerAll(_factory, _action_factory, _syntax);

  _the_warehouse = libmesh_make_unique<TheWarehouse>();
  _the_warehouse->registerAttribute<AttribMatrixTags>("matrix_tags", 0);
  _the_warehouse->registerAttribute<AttribVectorTags>("vector_tags", 0);
  _the_warehouse->registerAttribute<AttribExecOns>("exec_ons", 0);
  _the_warehouse->registerAttribute<AttribSubdomains>("subdomains", 0);
  _the_warehouse->registerAttribute<AttribBoundaries>("boundaries", 0);
  _the_warehouse->registerAttribute<AttribThread>("thread", 0);
  _the_warehouse->registerAttribute<AttribPreIC>("pre_ic", 0);
  _the_warehouse->registerAttribute<AttribPreAux>("pre_aux", 0);
  _the_warehouse->registerAttribute<AttribPostAux>("post_aux", 0);
  _the_warehouse->registerAttribute<AttribName>("name", "dummy");
  _the_warehouse->registerAttribute<AttribSystem>("system", "dummy");
  _the_warehouse->registerAttribute<AttribVar>("variable", 0);
  _the_warehouse->registerAttribute<AttribInterfaces>("interfaces", 0);

  if (isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char **>("_argv");

    _sys_info = libmesh_make_unique<SystemInfo>(argc, argv);
  }
  if (isParamValid("_command_line"))
    _command_line = getParam<std::shared_ptr<CommandLine>>("_command_line");
  else
    mooseError("Valid CommandLine object required");

  if (_check_input && isParamValid("recover"))
    mooseError("Cannot run --check-input with --recover. Recover files might not exist");

  if (isParamValid("start_in_debugger") && _multiapp_level == 0)
  {
    auto command = getParam<std::string>("start_in_debugger");

    Moose::out << "Starting in debugger using: " << command << std::endl;

    auto hostname = MooseUtils::hostname();

    std::stringstream command_stream;

    // This will start XTerm and print out some info first... then run the debugger
    command_stream << "xterm -e \"echo 'Rank: " << processor_id() << "  Hostname: " << hostname
                   << "  PID: " << getpid() << "'; echo ''; ";

    // Figure out how to run the debugger
    if (command.find("lldb") != std::string::npos || command.find("gdb") != std::string::npos)
      command_stream << command << " -p " << getpid();
    else
      mooseError("Unknown debugger: ",
                 command,
                 "\nIf this is truly what you meant then contact moose-users to have a discussion "
                 "about adding your debugger.");

    // Finish up the command
    command_stream << "\""
                   << " & ";

    std::string command_string = command_stream.str();
    Moose::out << "Running: " << command_string << std::endl;

    int ret = std::system(command_string.c_str());
    libmesh_ignore(ret);

    // Sleep to allow time for the debugger to attach
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  if (!parameters.isParamSetByAddParam("stop_for_debugger"))
  {
    Moose::out << "\nStopping for " << getParam<unsigned int>("stop_for_debugger")
               << " seconds to allow attachment from a debugger.\n";

    Moose::out << "\nAll of the processes you can connect to:\n";
    Moose::out << "rank - hostname - pid\n";

    auto hostname = MooseUtils::hostname();

    {
      // The 'false' turns off the serialization warning
      SerializerGuard sg(_communicator, false); // Guarantees that the processors print in order
      Moose::err << processor_id() << " - " << hostname << " - " << getpid() << "\n";
    }

    Moose::out << "\nWaiting...\n" << std::endl;

    // Sleep to allow time for the debugger to attach
    std::this_thread::sleep_for(std::chrono::seconds(getParam<unsigned int>("stop_for_debugger")));
  }

  if (_master_mesh && _multiapp_level == 0)
    mooseError("Mesh can be passed in only for sub-apps");

  if (_master_displaced_mesh && !_master_mesh)
    mooseError("_master_mesh should have been set when _master_displaced_mesh is set");

  // Data specifically associated with the mesh (meta-data) that will read from the restart
  // file early during the simulation setup so that they are available to Actions and other objects
  // that need them during the setup process. Most of the restartable data isn't made available
  // until all objects have been created and all Actions have been executed (i.e. initialSetup).
  registerRestartableDataMapName(MooseApp::MESH_META_DATA, "mesh");
}

void
MooseApp::checkRegistryLabels()
{
  Registry::checkLabels();
}

MooseApp::~MooseApp()
{
#ifdef HAVE_GPERFTOOLS
  if (_profiling)
    ProfilerStop();
#endif
  _action_warehouse.clear();
  _executioner.reset();
  _the_warehouse.reset();

  delete _input_parameter_warehouse;

#ifdef LIBMESH_HAVE_DLOPEN
  // Close any open dynamic libraries
  for (const auto & it : _lib_handles)
    dlclose(it.second);
#endif
}

std::string
MooseApp::getFrameworkVersion() const
{
  return MOOSE_VERSION;
}

std::string
MooseApp::getVersion() const
{
  return MOOSE_VERSION;
}

std::string
MooseApp::getPrintableVersion() const
{
  return getPrintableName() + " Version: " + getVersion();
}

void
MooseApp::setupOptions()
{
  TIME_SECTION(_setup_options_timer);

  // MOOSE was updated to have the ability to register execution flags in similar fashion as
  // objects. However, this change requires all *App.C/h files to be updated with the new
  // registerExecFlags method. To avoid breaking all applications the default MOOSE flags
  // are added if nothing has been added to this point. In the future this could go away or
  // perhaps be a warning.
  if (_execute_flags.items().empty())
    Moose::registerExecFlags(_factory);

  // Print the header, this is as early as possible
  std::string hdr(header() + "\n");
  if (multiAppLevel() > 0)
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

  // The no_timing flag takes precedence over the timing flag.
  if (getParam<bool>("no_timing"))
  {
    _pars.set<bool>("timing") = false;

    _perf_graph.setActive(false);
  }

  if (isParamValid("trap_fpe") && isParamValid("no_trap_fpe"))
    mooseError("Cannot use both \"--trap-fpe\" and \"--no-trap-fpe\" flags.");
  if (isParamValid("trap_fpe"))
    _trap_fpe = true;
  else if (isParamValid("no_trap_fpe"))
    _trap_fpe = false;

  // Turn all warnings in MOOSE to errors (almost see next logic block)
  Moose::_warnings_are_errors = getParam<bool>("error");

  // Deprecated messages can be toggled to errors independently from everything else.
  Moose::_deprecated_is_error = getParam<bool>("error_deprecated");

  if (isUltimateMaster()) // makes sure coloring isn't reset incorrectly in multi-app settings
  {
    // Toggle the color console off
    Moose::setColorConsole(true, true); // set default color condition
    if (getParam<bool>("no_color"))
      Moose::setColorConsole(false);

    char * c_color = std::getenv("MOOSE_COLOR");
    std::string color = "on";
    if (c_color)
      color = c_color;
    if (getParam<std::string>("color") != "default-on")
      color = getParam<std::string>("color");

    if (color == "auto")
      Moose::setColorConsole(true);
    else if (color == "on")
      Moose::setColorConsole(true, true);
    else if (color == "off")
      Moose::setColorConsole(false);
    else
      mooseWarning("ignoring invalid --color arg (want 'auto', 'on', or 'off')");
  }

  // this warning goes below --color processing to honor that setting for
  // the warning. And below settings for warnings/error setup.
  if (getParam<bool>("no_color"))
    mooseDeprecated("The --no-color flag is deprecated. Use '--color off' instead.");

// If there's no threading model active, but the user asked for
// --n-threads > 1 on the command line, throw a mooseError.  This is
// intended to prevent situations where the user has potentially
// built MOOSE incorrectly (neither TBB nor pthreads found) and is
// asking for multiple threads, not knowing that there will never be
// any threads launched.
#if !LIBMESH_USING_THREADS
  if (libMesh::command_line_value("--n-threads", 1) > 1)
    mooseError("You specified --n-threads > 1, but there is no threading model active!");
#endif

  // Build a minimal running application, ignoring the input file.
  if (getParam<bool>("minimal"))
    createMinimalApp();

  else if (getParam<bool>("display_version"))
  {
    Moose::perf_log.disable_logging();
    Moose::out << getPrintableVersion() << std::endl;
    _ready_to_exit = true;
    return;
  }
  else if (getParam<bool>("help"))
  {
    Moose::perf_log.disable_logging();

    _command_line->printUsage();
    _ready_to_exit = true;
  }
  else if (isParamValid("dump"))
  {
    Moose::perf_log.disable_logging();

    // Get command line argument following --dump on command line
    std::string following_arg = getParam<std::string>("dump");

    // The argument following --dump is a parameter search string,
    // which can be empty.
    std::string param_search;
    if (!following_arg.empty() && (following_arg.find('-') != 0))
      param_search = following_arg;

    JsonSyntaxTree tree(param_search);
    _parser.buildJsonSyntaxTree(tree);
    JsonInputFileFormatter formatter;
    Moose::out << "### START DUMP DATA ###\n"
               << formatter.toString(tree.getRoot()) << "\n### END DUMP DATA ###\n";
    _ready_to_exit = true;
  }
  else if (isParamValid("registry"))
  {
    Moose::out << "Label\tType\tName\tClass\tFile\n";

    auto & objmap = Registry::allObjects();
    for (auto & entry : objmap)
    {
      for (auto & obj : entry.second)
      {
        std::string name = obj._name;
        if (name.empty())
          name = obj._alias;
        if (name.empty())
          name = obj._classname;

        Moose::out << entry.first << "\tobject\t" << name << "\t" << obj._classname << "\t"
                   << obj._file << "\n";
      }
    }

    auto & actmap = Registry::allActions();
    for (auto & entry : actmap)
    {
      for (auto & act : entry.second)
        Moose::out << entry.first << "\taction\t" << act._name << "\t" << act._classname << "\t"
                   << act._file << "\n";
    }

    _ready_to_exit = true;
  }
  else if (isParamValid("registry_hit"))
  {
    Moose::out << "### START REGISTRY DATA ###\n";

    hit::Section root("");
    auto sec = new hit::Section("registry");
    root.addChild(sec);
    auto objsec = new hit::Section("objects");
    sec->addChild(objsec);

    auto & objmap = Registry::allObjects();
    for (auto & entry : objmap)
    {
      for (auto & obj : entry.second)
      {
        std::string name = obj._name;
        if (name.empty())
          name = obj._alias;
        if (name.empty())
          name = obj._classname;

        auto ent = new hit::Section("entry");
        objsec->addChild(ent);
        ent->addChild(new hit::Field("label", hit::Field::Kind::String, entry.first));
        ent->addChild(new hit::Field("type", hit::Field::Kind::String, "object"));
        ent->addChild(new hit::Field("name", hit::Field::Kind::String, name));
        ent->addChild(new hit::Field("class", hit::Field::Kind::String, obj._classname));
        ent->addChild(new hit::Field("file", hit::Field::Kind::String, obj._file));
      }
    }

    auto actsec = new hit::Section("actions");
    sec->addChild(actsec);
    auto & actmap = Registry::allActions();
    for (auto & entry : actmap)
    {
      for (auto & act : entry.second)
      {
        auto ent = new hit::Section("entry");
        actsec->addChild(ent);
        ent->addChild(new hit::Field("label", hit::Field::Kind::String, entry.first));
        ent->addChild(new hit::Field("type", hit::Field::Kind::String, "action"));
        ent->addChild(new hit::Field("task", hit::Field::Kind::String, act._name));
        ent->addChild(new hit::Field("class", hit::Field::Kind::String, act._classname));
        ent->addChild(new hit::Field("file", hit::Field::Kind::String, act._file));
      }
    }

    Moose::out << root.render();

    Moose::out << "\n### END REGISTRY DATA ###\n";
    _ready_to_exit = true;
  }
  else if (isParamValid("definition"))
  {
    Moose::perf_log.disable_logging();
    JsonSyntaxTree tree("");
    _parser.buildJsonSyntaxTree(tree);
    SONDefinitionFormatter formatter;
    Moose::out << formatter.toString(tree.getRoot()) << "\n";
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

    // Get command line argument following --json on command line
    std::string json_following_arg = getParam<std::string>("json");

    // The argument following --json is a parameter search string,
    // which can be empty.
    std::string search;
    if (!json_following_arg.empty() && (json_following_arg.find('-') != 0))
      search = json_following_arg;

    JsonSyntaxTree tree(search);
    _parser.buildJsonSyntaxTree(tree);

    Moose::out << "**START JSON DATA**\n" << tree.getRoot() << "\n**END JSON DATA**\n";
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
  else if (getParam<bool>("apptype"))
  {
    Moose::perf_log.disable_logging();
    Moose::out << "MooseApp Type: " << type() << std::endl;
    _ready_to_exit = true;
  }
  else if (_input_filename != "" ||
           isParamValid("input_file")) // They already specified an input filename
  {
    if (_input_filename == "")
      _input_filename = getParam<std::string>("input_file");

    if (isParamValid("recover"))
    {
      // We need to set the flag manually here since the recover parameter is a string type (takes
      // an optional filename)
      _recover = true;

      // Get command line argument following --recover on command line
      std::string recover_following_arg = getParam<std::string>("recover");

      // If the argument following --recover is non-existent or begins with
      // a dash then we are going to eventually find the newest recovery file to use
      if (!(recover_following_arg.empty() || (recover_following_arg.find('-') == 0)))
        _restart_recover_base = recover_following_arg;
    }

    // Optionally get command line argument following --recoversuffix
    // on command line.  Currently this argument applies to both
    // recovery and restart files.
    if (isParamValid("recoversuffix"))
    {
      _restart_recover_suffix = getParam<std::string>("recoversuffix");
    }

    _parser.parse(_input_filename);

    if (isParamValid("mesh_only"))
    {
      _syntax.registerTaskName("mesh_only", true);
      _syntax.addDependency("mesh_only", "setup_mesh_complete");
      _action_warehouse.setFinalTask("mesh_only");
    }
    else if (isParamValid("split_mesh"))
    {
      _split_mesh = true;
      _syntax.registerTaskName("split_mesh", true);
      _syntax.addDependency("split_mesh", "setup_mesh_complete");
      _action_warehouse.setFinalTask("split_mesh");
    }
    _action_warehouse.build();

    // Setup the AppFileBase for use by the Outputs or other systems that need output file info
    {
      // Extract the CommonOutputAction
      const auto & common_actions = _action_warehouse.getActionListByName("common_output");
      mooseAssert(common_actions.size() == 1, "Should be only one common_output Action");

      const Action * common = *common_actions.begin();

      // If file_base is set in CommonOutputAction through parsing input, obtain the file_base
      if (common->isParamValid("file_base"))
      {
        _output_file_base = common->getParam<std::string>("file_base");
        _file_base_set_by_user = true;
      }
      else if (isUltimateMaster())
      {
        // if this app is a master, we use the input file name as the default file base
        std::string base = getInputFileName();
        size_t pos = base.find_last_of('.');
        _output_file_base = base.substr(0, pos);
        // Note: we did not append "_out" in the file base here because we do not want to
        //       have it in betwen the input file name and the object name for Output/*
        //       syntax.
      }
      // default file base for multiapps is set by MultiApp
    }
  }
  else /* The catch-all case for bad options or missing options, etc. */
  {
    Moose::perf_log.disable_logging();

    if (_check_input)
      mooseError("You specified --check-input, but did not provide an input file. Add -i "
                 "<inputfile> to your command line.");

    _command_line->printUsage();
    _ready_to_exit = true;
  }
}

void
MooseApp::setInputFileName(const std::string & input_filename)
{
  _input_filename = input_filename;
}

std::string
MooseApp::getOutputFileBase(bool for_non_moose_build_output) const
{
  if (_file_base_set_by_user || for_non_moose_build_output || _multiapp_level)
    return _output_file_base;
  else
    return _output_file_base + "_out";
}

void
MooseApp::runInputFile()
{
  TIME_SECTION(_run_input_file_timer);

  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  _action_warehouse.executeAllActions();

  if (isParamValid("mesh_only") || isParamValid("split_mesh"))
    _ready_to_exit = true;
  else if (getParam<bool>("list_constructed_objects"))
  {
    // TODO: ask multiapps for their constructed objects
    _ready_to_exit = true;
    std::vector<std::string> obj_list = _factory.getConstructedObjects();
    Moose::out << "**START OBJECT DATA**\n";
    for (const auto & name : obj_list)
      Moose::out << name << "\n";
    Moose::out << "**END OBJECT DATA**\n" << std::endl;
  }
}

void
MooseApp::errorCheck()
{
  bool warn = _enable_unused_check == WARN_UNUSED;
  bool err = _enable_unused_check == ERROR_UNUSED;

  _parser.errorCheck(*_comm, warn, err);

  auto apps = _executioner->feProblem().getMultiAppWarehouse().getObjects();
  for (auto app : apps)
    for (unsigned int i = 0; i < app->numLocalApps(); i++)
      app->localApp(i)->errorCheck();
}

void
MooseApp::executeExecutioner()
{
  TIME_SECTION(_execute_executioner_timer);

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
    errorCheck();
    _executioner->execute();
  }
  else
    mooseError("No executioner was specified (go fix your input file)");
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
MooseApp::isSplitMesh() const
{
  return _split_mesh;
}

bool
MooseApp::isUseSplit() const
{
  return _use_split;
}

bool
MooseApp::hasRestartRecoverFileBase() const
{
  return !_restart_recover_base.empty();
}

bool
MooseApp::hasRecoverFileBase() const
{
  mooseDeprecated("MooseApp::hasRecoverFileBase is deprecated, use "
                  "MooseApp::hasRestartRecoverFileBase() instead.");
  return !_restart_recover_base.empty();
}

void
MooseApp::registerRestartableNameWithFilter(const std::string & name,
                                            Moose::RESTARTABLE_FILTER filter)
{
  using Moose::RESTARTABLE_FILTER;
  switch (filter)
  {
    case RESTARTABLE_FILTER::RECOVERABLE:
      _recoverable_data_names.insert(name);
      break;
    default:
      mooseError("Unknown filter");
  }
}

std::shared_ptr<Backup>
MooseApp::backup()
{
  mooseAssert(_executioner, "Executioner is nullptr");
  FEProblemBase & fe_problem = _executioner->feProblem();

  RestartableDataIO rdio(fe_problem);
  return rdio.createBackup();
}

void
MooseApp::restore(std::shared_ptr<Backup> backup, bool for_restart)
{
  TIME_SECTION(_restore_timer);

  mooseAssert(_executioner, "Executioner is nullptr");
  FEProblemBase & fe_problem = _executioner->feProblem();

  RestartableDataIO rdio(fe_problem);
  rdio.restoreBackup(backup, for_restart);
}

void
MooseApp::setCheckUnusedFlag(bool warn_is_error)
{
  /**
   * _enable_unused_check is initialized to WARN_UNUSED. If an application chooses to promote
   * this value to ERROR_UNUSED programmatically prior to running the simulation, we certainly
   * don't want to allow it to fall back. Therefore, we won't set it if it's already at the
   * highest value (i.e. error). If however a developer turns it off, it can still be turned on.
   */
  if (_enable_unused_check != ERROR_UNUSED || warn_is_error)
    _enable_unused_check = warn_is_error ? ERROR_UNUSED : WARN_UNUSED;
  else
    mooseInfo("Ignoring request to turn off or warn about unused parameters.\n");
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

void
MooseApp::run()
{
  TIME_SECTION(_run_timer);

  try
  {
    TIME_SECTION(_setup_timer);
    setupOptions();
    runInputFile();
  }
  catch (std::exception & err)
  {
    mooseError(err.what());
  }

  if (!_check_input)
  {
    TIME_SECTION(_execute_timer);
    executeExecutioner();
  }
  else
  {
    errorCheck();
    // Output to stderr, so it is easier for peacock to get the result
    Moose::err << "Syntax OK" << std::endl;
  }
}

void
MooseApp::setOutputPosition(const Point & p)
{
  _output_position_set = true;
  _output_position = p;
  _output_warehouse.meshChanged();

  if (_executioner.get())
    _executioner->parentOutputPositionChanged();
}

std::list<std::string>
MooseApp::getCheckpointDirectories() const
{
  // Storage for the directory names
  std::list<std::string> checkpoint_dirs;

  // Add the directories added with Outputs/checkpoint=true input syntax
  checkpoint_dirs.push_back(getOutputFileBase() + "_cp");

  // Add the directories from any existing checkpoint output objects
  const auto & actions = _action_warehouse.getActionListByName("add_output");
  for (const auto & action : actions)
  {
    // Get the parameters from the MooseObjectAction
    MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    if (!moose_object_action)
      continue;

    const InputParameters & params = moose_object_action->getObjectParams();
    if (moose_object_action->getParam<std::string>("type") == "Checkpoint")
      checkpoint_dirs.push_back(params.get<std::string>("file_base") + "_cp");
  }

  return checkpoint_dirs;
}

std::list<std::string>
MooseApp::getCheckpointFiles() const
{
  auto checkpoint_dirs = getCheckpointDirectories();
  return MooseUtils::getFilesInDirs(checkpoint_dirs);
}

void
MooseApp::setStartTime(Real time)
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
    mooseError("Invalid application name: ", library_name);
  library_name.erase(pos);

  // Now get rid of the camel case, prepend lib, and append the method and suffix
  return std::string("lib") + MooseUtils::camelCaseToUnderscore(library_name) + '-' +
         QUOTE(METHOD) + ".la";
}

std::string
MooseApp::libNameToAppName(const std::string & library_name) const
{
  std::string app_name(library_name);

  // Strip off the leading "lib" and trailing ".la"
  if (pcrecpp::RE("lib(.+?)(?:-\\w+)?\\.la").Replace("\\1", &app_name) == 0)
    mooseError("Invalid library name: ", app_name);

  return MooseUtils::underscoreToCamelCase(app_name, true);
}

RestartableDataValue &
MooseApp::registerRestartableData(const std::string & name,
                                  std::unique_ptr<RestartableDataValue> data,
                                  THREAD_ID tid,
                                  bool read_only,
                                  const RestartableDataMapName & metaname)
{
  if (!metaname.empty() && tid != 0)
    mooseError(
        "The meta data storage for '", metaname, "' is not threaded, so the tid must be zero.");

  mooseAssert(metaname.empty() ||
                  _restartable_meta_data.find(metaname) != _restartable_meta_data.end(),
              "The desired meta data name does not exist: " + metaname);

  // Select the data store for saving this piece of restartable data (mesh or everything else)
  auto & data_ref =
      metaname.empty() ? _restartable_data[tid] : _restartable_meta_data[metaname].first;

  auto insert_pair = data_ref.emplace(name, RestartableDataValuePair(std::move(data), !read_only));

  // Does the storage for this data already exist?
  if (!insert_pair.second)
  {
    auto & data = insert_pair.first->second;

    // Are we really declaring or just trying to get a reference to the data?
    if (!read_only)
    {
      if (data.declared)
        mooseError("Attempted to declare restartable mesh meta data twice with the same name: ",
                   name);
      else
        // The data wasn't previously declared, but now it is!
        data.declared = true;
    }
  }

  return *insert_pair.first->second.value;
}

void
MooseApp::dynamicAppRegistration(const std::string & app_name,
                                 std::string library_path,
                                 const std::string & library_name)
{
#ifdef LIBMESH_HAVE_DLOPEN
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = APPLICATION;
  params.set<std::string>("registration_method") = app_name + "__registerApps";
  params.set<std::string>("library_path") = library_path;
  params.set<std::string>("library_name") = library_name;

  dynamicRegistration(params);

  // At this point the application should be registered so check it
  if (!AppFactory::instance().isRegistered(app_name))
  {
    std::ostringstream oss;
    std::set<std::string> paths = getLoadedLibraryPaths();

    oss << "Unable to locate library for \"" << app_name
        << "\".\nWe attempted to locate the library \"" << appNameToLibName(app_name)
        << "\" in the following paths:\n\t";
    std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
    oss << "\n\nMake sure you have compiled the library and either set the \"library_path\" "
           "variable "
        << "in your input file or exported \"MOOSE_LIBRARY_PATH\".\n"
        << "Compiled in debug mode to see the list of libraries checked for dynamic loading "
           "methods.";
    mooseError(oss.str());
  }
#else
  mooseError("Dynamic Loading is either not supported or was not detected by libMesh configure.");
#endif
}

void
MooseApp::dynamicAllRegistration(const std::string & app_name,
                                 Factory * factory,
                                 ActionFactory * action_factory,
                                 Syntax * syntax,
                                 std::string library_path,
                                 const std::string & library_name)
{
#ifdef LIBMESH_HAVE_DLOPEN
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = REGALL;
  params.set<std::string>("registration_method") = app_name + "__registerAll";
  params.set<std::string>("library_path") = library_path;
  params.set<std::string>("library_name") = library_name;

  params.set<Factory *>("factory") = factory;
  params.set<Syntax *>("syntax") = syntax;
  params.set<ActionFactory *>("action_factory") = action_factory;

  dynamicRegistration(params);
#else
  mooseError("Dynamic Loading is either not supported or was not detected by libMesh configure.");
#endif
}

void
MooseApp::dynamicRegistration(const Parameters & params)
{
  std::string library_name;
  // was library name provided by the user?
  if (params.get<std::string>("library_name").empty())
    library_name = appNameToLibName(params.get<std::string>("app_name"));
  else
    library_name = params.get<std::string>("library_name");

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
      mooseWarning("Unable to open library file \"",
                   path + '/' + library_name,
                   "\". Double check for spelling errors.");
}

void
MooseApp::loadLibraryAndDependencies(const std::string & library_filename,
                                     const Parameters & params)
{
  std::string line;
  std::string dl_lib_filename;

  // This RE looks for absolute path libtool filenames (i.e. begins with a slash and ends with a
  // .la)
  pcrecpp::RE re_deps("(/\\S*\\.la)");

  std::ifstream handle(library_filename.c_str());
  if (handle.is_open())
  {
    while (std::getline(handle, line))
    {
      // Look for the system dependent dynamic library filename to open
      if (line.find("dlname=") != std::string::npos)
        // Magic numbers are computed from length of this string "dlname=' and line minus that
        // string plus quotes"
        dl_lib_filename = line.substr(8, line.size() - 9);

      if (line.find("dependency_libs=") != std::string::npos)
      {
        pcrecpp::StringPiece input(line);
        pcrecpp::StringPiece depend_library;
        while (re_deps.FindAndConsume(&input, &depend_library))
          // Recurse here to load dependent libraries in depth-first order
          loadLibraryAndDependencies(depend_library.as_string(), params);

        // There's only one line in the .la file containing the dependency libs so break after
        // finding it
        break;
      }
    }
    handle.close();
  }

  std::string registration_method_name = params.get<std::string>("registration_method");
  // Time to load the library, First see if we've already loaded this particular dynamic library
  if (_lib_handles.find(std::make_pair(library_filename, registration_method_name)) ==
          _lib_handles.end() && // make sure we haven't already loaded this library
      dl_lib_filename != "") // AND make sure we have a library name (we won't for static linkage)
  {
    std::pair<std::string, std::string> lib_name_parts =
        MooseUtils::splitFileName(library_filename);

    // Assemble the actual filename using the base path of the *.la file and the dl_lib_filename
    std::string dl_lib_full_path = lib_name_parts.first + '/' + dl_lib_filename;

    MooseUtils::checkFileReadable(dl_lib_full_path, false, /*throw_on_unreadable=*/true);

#ifdef LIBMESH_HAVE_DLOPEN
    void * handle = dlopen(dl_lib_full_path.c_str(), RTLD_LAZY);
#else
    void * handle = nullptr;
#endif

    if (!handle)
      mooseError("The library file \"",
                 dl_lib_full_path,
                 "\" exists and has proper permissions, but cannot by dynamically loaded.\nThis "
                 "generally means that the loader was unable to load one or more of the "
                 "dependencies listed in the supplied library (see otool or ldd).\n");

// get the pointer to the method in the library.  The dlsym()
// function returns a null pointer if the symbol cannot be found,
// we also explicitly set the pointer to NULL if dlsym is not
// available.
#ifdef LIBMESH_HAVE_DLOPEN
    void * registration_method = dlsym(handle, registration_method_name.c_str());
#else
    void * registration_method = nullptr;
#endif

    if (!registration_method)
    {
// We found a dynamic library that doesn't have a dynamic
// registration method in it. This shouldn't be an error, so
// we'll just move on.
#ifdef DEBUG
      mooseWarning("Unable to find extern \"C\" method \"",
                   registration_method_name,
                   "\" in library: ",
                   dl_lib_full_path,
                   ".\n",
                   "This doesn't necessarily indicate an error condition unless you believe that "
                   "the method should exist in that library.\n");
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
          register_app_t * reg_ptr = reinterpret_cast<register_app_t *>(&registration_method);
          (*reg_ptr)();
          break;
        }
        case REGALL:
        {
          typedef void (*register_app_t)(Factory *, ActionFactory *, Syntax *);
          register_app_t * reg_ptr = reinterpret_cast<register_app_t *>(&registration_method);
          (*reg_ptr)(params.get<Factory *>("factory"),
                     params.get<ActionFactory *>("action_factory"),
                     params.get<Syntax *>("syntax"));
          break;
        }
        default:
          mooseError("Unhandled RegistrationType");
      }

      // Store the handle so we can close it later
      _lib_handles.insert(
          std::make_pair(std::make_pair(library_filename, registration_method_name), handle));
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
MooseApp::addMeshModifier(const std::string & modifier_name,
                          const std::string & name,
                          InputParameters parameters)
{
  std::shared_ptr<MeshModifier> mesh_modifier =
      _factory.create<MeshModifier>(modifier_name, name, parameters);

  _mesh_modifiers.insert(std::make_pair(MooseUtils::shortName(name), mesh_modifier));
}

const MeshModifier &
MooseApp::getMeshModifier(const std::string & name) const
{
  return *_mesh_modifiers.find(MooseUtils::shortName(name))->second.get();
}

std::vector<std::string>
MooseApp::getMeshModifierNames() const
{
  std::vector<std::string> names;
  for (auto & pair : _mesh_modifiers)
    names.push_back(pair.first);
  return names;
}

void
MooseApp::executeMeshModifiers()
{
  if (!_mesh_modifiers.empty())
  {
    TIME_SECTION(_execute_mesh_modifiers_timer);

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
          mooseError("The MeshModifier \"",
                     depend_name,
                     "\" was not created, did you make a "
                     "spelling mistake or forget to include it "
                     "in your input file?");

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
       * Set preparation flag after modifiers are run. The final preparation
       * will be handled by the SetupMeshComplete Action.
       */
      mesh->prepared(false);
      if (displaced_mesh)
        displaced_mesh->prepared(false);
    }
  }
}

void
MooseApp::clearMeshModifiers()
{
  _mesh_modifiers.clear();
}

void
MooseApp::addMeshGenerator(const std::string & generator_name,
                           const std::string & name,
                           InputParameters parameters)
{
  std::shared_ptr<MeshGenerator> mesh_generator =
      _factory.create<MeshGenerator>(generator_name, name, parameters);

  _mesh_generators.insert(std::make_pair(MooseUtils::shortName(name), mesh_generator));
}

const MeshGenerator &
MooseApp::getMeshGenerator(const std::string & name) const
{
  return *_mesh_generators.find(MooseUtils::shortName(name))->second.get();
}

std::vector<std::string>
MooseApp::getMeshGeneratorNames() const
{
  std::vector<std::string> names;
  for (auto & pair : _mesh_generators)
    names.push_back(pair.first);
  return names;
}

std::unique_ptr<MeshBase> &
MooseApp::getMeshGeneratorOutput(const std::string & name)
{
  auto & outputs = _mesh_generator_outputs[name];

  outputs.push_back(nullptr);

  return outputs.back();
}

void
MooseApp::createMeshGeneratorOrder()
{
  // we only need to create the order once
  if (_ordered_generators.size() > 0)
    return;

  TIME_SECTION(_execute_mesh_generators_timer);

  DependencyResolver<std::shared_ptr<MeshGenerator>> resolver;

  // Add all of the dependencies into the resolver and sort them
  for (const auto & it : _mesh_generators)
  {
    // Make sure an item with no dependencies comes out too!
    resolver.addItem(it.second);

    std::vector<std::string> & generators = it.second->getDependencies();
    for (const auto & depend_name : generators)
    {
      auto depend_it = _mesh_generators.find(depend_name);

      if (depend_it == _mesh_generators.end())
        mooseError("The MeshGenerator \"",
                   depend_name,
                   "\" was not created, did you make a "
                   "spelling mistake or forget to include it "
                   "in your input file?");

      resolver.insertDependency(it.second, depend_it->second);
    }
  }

  _ordered_generators = resolver.getSortedValuesSets();

  if (_ordered_generators.size())
  {
    auto & final_generators = _ordered_generators.back();

    if (_final_generator_name.empty())
    {
      // If the _final_generated_mesh wasn't set from MeshGeneratorMesh, set it now
      _final_generator_name = final_generators.back()->name();

      // See if we have multiple independent trees of generators
      const auto ancestor_list = resolver.getAncestors(final_generators.back());
      if (ancestor_list.size() != resolver.size())
      {
        // Need to remove duplicates and possibly perform a difference so we'll import out list
        // into a set for these operations.
        std::set<std::shared_ptr<MeshGenerator>> ancestors(ancestor_list.begin(),
                                                           ancestor_list.end());
        // Get all of the items from the resolver so we can compare against the tree from the
        // final generator we just pulled.
        const auto & allValues = resolver.getSortedValues();
        decltype(ancestors) all(allValues.begin(), allValues.end());

        decltype(ancestors) ind_tree;
        std::set_difference(all.begin(),
                            all.end(),
                            ancestors.begin(),
                            ancestors.end(),
                            std::inserter(ind_tree, ind_tree.end()));

        std::ostringstream oss;
        oss << "Your MeshGenerator tree contains multiple possible generator outputs :\n\""
            << _final_generator_name
            << " and one or more of the following from an independent set: \"";
        bool first = true;
        for (const auto & gen : ind_tree)
        {
          if (!first)
            oss << ", ";
          else
            first = false;

          oss << gen->name();
        }
        oss << "\"\n\nThis may be due to a missing dependency or may be intentional. Please "
               "select the final MeshGenerator in\nthe [Mesh] block with the \"final_generator\" "
               "parameter or add additional dependencies to remove the ambiguity.";
        mooseError(oss.str());
      }
    }
  }
}

void
MooseApp::appendMeshGenerator(const std::string & generator_name,
                              const std::string & name,
                              InputParameters parameters)
{
  if (_mesh_generators.empty())
    mooseError("Cannot append a mesh generator because no mesh generators exist");

  if (!parameters.have_parameter<MeshGeneratorName>("input"))
    mooseError("Cannot append a mesh generator that does not take input mesh generators");

  createMeshGeneratorOrder();

  auto & final_generators = _ordered_generators.back();

  // set the final generator as the input
  if (_final_generator_name.empty())
    parameters.set<MeshGeneratorName>("input") = final_generators.back()->name();
  else
  {
    parameters.set<MeshGeneratorName>("input") = _final_generator_name;
    _final_generator_name = name;
  }

  std::shared_ptr<MeshGenerator> mesh_generator =
      _factory.create<MeshGenerator>(generator_name, name, parameters);

  final_generators.push_back(mesh_generator);
}

void
MooseApp::executeMeshGenerators()
{
  // we do not need to do this when there are no mesh generators
  if (_mesh_generators.empty())
    return;

  createMeshGeneratorOrder();

  // set the final generator name
  auto & final_generators = _ordered_generators.back();
  if (_final_generator_name.empty())
    _final_generator_name = final_generators.back()->name();

  // Grab the outputs from the final generator so MeshGeneratorMesh can pick them up
  _final_generated_meshes.emplace_back(&getMeshGeneratorOutput(_final_generator_name));

  // Need to grab two if we're going to be making a displaced mesh
  if (_action_warehouse.displacedMesh())
    _final_generated_meshes.emplace_back(&getMeshGeneratorOutput(_final_generator_name));

  // Run the MeshGenerators in the proper order
  for (const auto & generator_set : _ordered_generators)
  {
    for (const auto & generator : generator_set)
    {
      auto name = generator->name();

      auto current_mesh = generator->generate();

      // Now we need to possibly give this mesh to downstream generators
      auto & outputs = _mesh_generator_outputs[name];

      if (outputs.size())
      {
        auto & first_output = *outputs.begin();

        first_output = std::move(current_mesh);

        const auto & copy_from = *first_output;

        auto output_it = ++outputs.begin();

        // For all of the rest we need to make a copy
        for (; output_it != outputs.end(); ++output_it)
          (*output_it) = copy_from.clone();
      }

      // Once we hit the generator we want, we'll terminate the loops (this might be the last
      // iteration anyway)
      if (_final_generator_name == name)
        return;
    }
  }
}

void
MooseApp::setFinalMeshGeneratorName(const std::string & generator_name)
{
  _final_generator_name = generator_name;
}

void
MooseApp::clearMeshGenerators()
{
  _ordered_generators.clear();
  _mesh_generators.clear();
}

std::unique_ptr<MeshBase>
MooseApp::getMeshGeneratorMesh(bool check_unique)
{
  if (_popped_final_mesh_generator == true)
    mooseError("MooseApp::getMeshGeneratorMesh is being called for a second time. You cannot do "
               "this because the final generated mesh was popped from its storage container the "
               "first time this method was called");

  if (_final_generated_meshes.empty())
    mooseError("No generated mesh to retrieve. Your input file should contain either a [Mesh] or "
               "[MeshGenerators] block.");

  auto mesh_unique_ptr_ptr = _final_generated_meshes.front();
  _final_generated_meshes.pop_front();
  _popped_final_mesh_generator = true;

  if (check_unique && !_final_generated_meshes.empty())
    mooseError("Multiple generated meshes exist while retrieving the final Mesh. This means that "
               "the selection of the final mesh is non-deterministic.");

  return std::move(*mesh_unique_ptr_ptr);
}

void
MooseApp::setRestart(bool value)
{
  _restart = value;
}

void
MooseApp::setRecover(bool value)
{
  _recover = value;
}

void
MooseApp::setBackupObject(std::shared_ptr<Backup> backup)
{
  _cached_backup = backup;
}

void
MooseApp::restoreCachedBackup()
{
  if (!_cached_backup.get())
    mooseError("No cached Backup to restore!");

  TIME_SECTION(_restore_cached_backup_timer);

  restore(_cached_backup, isRestarting());

  // Release our hold on this Backup
  _cached_backup.reset();
}

void
MooseApp::createMinimalApp()
{
  TIME_SECTION(_create_minimal_app_timer);

  // SetupMeshAction
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");
    action_params.set<std::string>("type") = "GeneratedMesh";

    // Create The Action
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("SetupMeshAction", "Mesh", action_params));

    // Set the object parameters
    InputParameters & params = action->getObjectParams();
    params.set<MooseEnum>("dim") = "1";
    params.set<unsigned int>("nx") = 1;

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  // Executioner
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CreateExecutionerAction");
    action_params.set<std::string>("type") = "Transient";

    // Create the action
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("CreateExecutionerAction", "Executioner", action_params));

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
    InputParameters action_params = _action_factory.getValidParams("CreateProblemDefaultAction");
    action_params.set<bool>("_solve") = false;

    // Create the action
    std::shared_ptr<Action> action = std::static_pointer_cast<Action>(
        _action_factory.create("CreateProblemDefaultAction", "Problem", action_params));

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  // Outputs
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("CommonOutputAction");
    action_params.set<bool>("console") = false;

    // Create action
    std::shared_ptr<Action> action =
        _action_factory.create("CommonOutputAction", "Outputs", action_params);

    // Add Action to the warehouse
    _action_warehouse.addActionBlock(action);
  }

  _action_warehouse.build();
}

void
MooseApp::addExecFlag(const ExecFlagType & flag)
{
  if (flag.id() == MooseEnumItem::INVALID_ID)
  {
    // It is desired that users when creating ExecFlagTypes should not worry about needing
    // to assign a name and an ID. However, the ExecFlagTypes created by users are global
    // constants and the ID to be assigned can't be known at construction time of this global
    // constant, it is only known when it is added to this object (ExecFlagEnum). Therefore,
    // this const cast allows the ID to be set after construction. This was the lesser of two
    // evils: const_cast or friend class with mutable members.
    ExecFlagType & non_const_flag = const_cast<ExecFlagType &>(flag);
    auto it = _execute_flags.find(flag.name());
    if (it != _execute_flags.items().end())
      non_const_flag.setID(it->id());
    else
      non_const_flag.setID(_execute_flags.getNextValidID());
  }
  _execute_flags.addAvailableFlags(flag);
}

bool
MooseApp::hasRelationshipManager(const std::string & name) const
{
  return std::find_if(_relationship_managers.begin(),
                      _relationship_managers.end(),
                      [&name](const std::shared_ptr<RelationshipManager> & rm) {
                        return rm->name() == name;
                      }) != _relationship_managers.end();
}

bool
MooseApp::addRelationshipManager(std::shared_ptr<RelationshipManager> relationship_manager)
{
  // We don't need Geometric-only RelationshipManagers when we run with
  // ReplicatedMesh unless we are splitting the mesh.
  if (!_action_warehouse.mesh()->isDistributedMesh() && !_split_mesh &&
      (relationship_manager->isType(Moose::RelationshipManagerType::GEOMETRIC) &&
       !(relationship_manager->isType(Moose::RelationshipManagerType::ALGEBRAIC) ||
         relationship_manager->isType(Moose::RelationshipManagerType::COUPLING))))
    return false;

  bool add = true;
  for (const auto & rm : _relationship_managers)
  {
    if (*rm == *relationship_manager)
    {
      add = false;

      auto & existing_for_whom = rm->forWhom();

      // Since the existing object is going to cover this one
      // Pass along who is needing it
      for (auto & fw : relationship_manager->forWhom())
      {
        if (std::find(existing_for_whom.begin(), existing_for_whom.end(), fw) ==
            existing_for_whom.end())
          rm->addForWhom(fw);
      }

      break;
    }
  }

  if (add)
    _relationship_managers.emplace_back(relationship_manager);

  // Inform the caller whether the object was added or not
  return add;
}

void
MooseApp::attachRelationshipManagers(Moose::RelationshipManagerType rm_type)
{
  for (auto & rm : _relationship_managers)
  {
    if (rm->isType(rm_type))
    {
      // Will attach them later (during algebraic)
      if (rm_type == Moose::RelationshipManagerType::GEOMETRIC && !rm->attachGeometricEarly())
        continue;

      if (rm_type == Moose::RelationshipManagerType::GEOMETRIC)
      {
        // The problem is not built yet - so the ActionWarehouse currently owns the mesh
        auto & mesh = _action_warehouse.mesh();

        rm->init();

        if (rm->useDisplacedMesh() && _action_warehouse.displacedMesh())
          _action_warehouse.displacedMesh()->getMesh().add_ghosting_functor(*rm);
        else
          mesh->getMesh().add_ghosting_functor(*rm);
      }

      if (rm_type != Moose::RelationshipManagerType::GEOMETRIC)
      {
        // Now we've built the problem, so we can use it
        auto & problem = _executioner->feProblem();

        // Ensure that the relationship manager is initialized
        rm->init();

        // If it's also Geometric but didn't get attached early - then let's attach it now
        if (rm->isType(Moose::RelationshipManagerType::GEOMETRIC) && !rm->attachGeometricEarly())
        {
          if (rm->useDisplacedMesh() && _action_warehouse.displacedMesh())
            _action_warehouse.displacedMesh()->getMesh().add_ghosting_functor(*rm);
          else
            problem.mesh().getMesh().add_ghosting_functor(*rm);
        }

        if (rm->useDisplacedMesh() && problem.getDisplacedProblem())
        {
          if (rm_type == Moose::RelationshipManagerType::COUPLING)
          {
            // We actually need to add this to the FEProblemBase NonlinearSystemBase's DofMap
            // because the DisplacedProblem "nonlinear" DisplacedSystem doesn't have any matrices
            // for which to do coupling
            auto & dof_map = problem.getNonlinearSystemBase().dofMap();
            dof_map.add_coupling_functor(*rm, /*to_mesh = */ false);
            rm->setDofMap(dof_map);
          }
          // If this rm is algebraic AND coupling, then in the case of the non-linear system there
          // is no reason to add it to the DofMap twice. In the case of any other system, it
          // actually would be disastrous to add this rm because it's going to set a coupling matrix
          // based on the non-linear system. So we don't add this rm at all here if its also
          // a coupling functor
          else if (rm_type == Moose::RelationshipManagerType::ALGEBRAIC &&
                   !rm->isType(Moose::RelationshipManagerType::COUPLING))
            problem.getDisplacedProblem()->addAlgebraicGhostingFunctor(*rm, /*to_mesh = */ false);
        }
        else // undisplaced
        {
          if (rm_type == Moose::RelationshipManagerType::COUPLING)
          {
            auto & dof_map = problem.getNonlinearSystemBase().dofMap();
            dof_map.add_coupling_functor(*rm, /*to_mesh = */ false);
            rm->setDofMap(dof_map);
          }
          // If this rm is algebraic AND coupling, then in the case of the non-linear system there
          // is no reason to add it to the DofMap twice. In the case of any other system, it
          // actually would be disastrous to add this rm because it's going to set a coupling matrix
          // based on the non-linear system. So we don't add this rm at all here if its also
          // a coupling functor
          else if (rm_type == Moose::RelationshipManagerType::ALGEBRAIC &&
                   !rm->isType(Moose::RelationshipManagerType::COUPLING))
            problem.addAlgebraicGhostingFunctor(*rm, /*to_mesh = */ false);
        }
      }
    }
  }
}

std::vector<std::pair<std::string, std::string>>
MooseApp::getRelationshipManagerInfo() const
{
  std::vector<std::pair<std::string, std::string>> info_strings;
  info_strings.reserve(_relationship_managers.size());

  for (const auto & rm : _relationship_managers)
  {
    std::stringstream oss;
    oss << rm->getInfo();

    auto & for_whom = rm->forWhom();

    if (!for_whom.empty())
    {
      oss << " for ";

      std::copy(for_whom.begin(), for_whom.end(), infix_ostream_iterator<std::string>(oss, ", "));
    }

    info_strings.emplace_back(std::make_pair(Moose::stringify(rm->getType()), oss.str()));
  }

  // List the libMesh GhostingFunctors - Not that in libMesh all of the algebraic and coupling
  // Ghosting Functors are also attached to the mesh. This should catch them all.
  const auto & mesh = _action_warehouse.getMesh();
  if (mesh)
  {
    std::unordered_map<std::string, unsigned int> counts;

    for (auto & gf : as_range(mesh->getMesh().ghosting_functors_begin(),
                              mesh->getMesh().ghosting_functors_end()))
    {
      const auto * gf_ptr = dynamic_cast<const RelationshipManager *>(gf);
      if (!gf_ptr)
        // Count how many occurances of the same Ghosting Functor types we are encountering
        counts[demangle(typeid(*gf).name())]++;
    }

    for (const auto pair : counts)
      info_strings.emplace_back(std::make_pair(
          "Default", pair.first + (pair.second > 1 ? " x " + std::to_string(pair.second) : "")));
  }

  return info_strings;
}

void
MooseApp::dofMapReinitForRMs()
{
  for (auto & rm : _relationship_managers)
    rm->dofmap_reinit();
}

void
MooseApp::meshReinitForRMs()
{
  for (auto & rm : _relationship_managers)
    rm->mesh_reinit();
}

void
MooseApp::checkMetaDataIntegrity() const
{
  for (auto map_iter = _restartable_meta_data.begin(); map_iter != _restartable_meta_data.end();
       ++map_iter)
  {
    const RestartableDataMapName & name = map_iter->first;
    const RestartableDataMap & meta_data = map_iter->second.first;

    std::vector<std::string> not_declared;

    for (const auto & pair : meta_data)
      if (!pair.second.declared)
        not_declared.push_back(pair.first);

    if (!not_declared.empty())
    {
      std::ostringstream oss;
      std::copy(
          not_declared.begin(), not_declared.end(), infix_ostream_iterator<std::string>(oss, ", "));

      mooseError("The following '",
                 name,
                 "' meta-data properties were retrieved but never declared: ",
                 oss.str());
    }
  }
}

const RestartableDataMapName MooseApp::MESH_META_DATA = "MeshMetaData";

const RestartableDataMap &
MooseApp::getRestartableDataMap(const RestartableDataMapName & name) const
{
  auto iter = _restartable_meta_data.find(name);
  if (iter == _restartable_meta_data.end())
    mooseError("Unable to find RestartableDataMap object for the supplied name '",
               name,
               "', did you call registerRestartableDataMapName in the application constructor?");
  return iter->second.first;
}

void
MooseApp::registerRestartableDataMapName(const RestartableDataMapName & name, std::string suffix)
{
  if (suffix.empty())
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
  suffix.insert(0, "_");
  _restartable_meta_data.emplace(
      std::make_pair(name, std::make_pair(RestartableDataMap(), suffix)));
}
