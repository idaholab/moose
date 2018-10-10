//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseApp.h"
#include "MooseRevision.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
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
#include "JsonSyntaxTree.h"
#include "JsonInputFileFormatter.h"
#include "SONDefinitionFormatter.h"
#include "RelationshipManager.h"
#include "Registry.h"
#include "SerializerGuard.h"
#include "PerfGraphInterface.h" // For TIME_SECTIOn

// Regular expression includes
#include "pcrecpp.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/checkpoint_io.h"

// System include for dynamic library methods
#include <dlfcn.h>
#include <sys/utsname.h> // utsname

// C++ includes
#include <numeric> // std::accumulate
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib> // for system()
#include <chrono>
#include <thread>

#define QUOTE(macro) stringifyName(macro)

template <>
InputParameters
validParams<MooseApp>()
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

  params.addPrivateParam<std::string>("_app_name"); // the name passed to AppFactory::create
  params.addPrivateParam<std::string>("_type");
  params.addPrivateParam<int>("_argc");
  params.addPrivateParam<char **>("_argv");
  params.addPrivateParam<std::shared_ptr<CommandLine>>("_command_line");
  params.addPrivateParam<std::shared_ptr<Parallel::Communicator>>("_comm");
  params.addPrivateParam<unsigned int>("_multiapp_level");
  params.addPrivateParam<unsigned int>("_multiapp_number");

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
    _recover_suffix("cpr"),
    _half_transient(false),
    _check_input(getParam<bool>("check_input")),
    _restartable_data(libMesh::n_threads()),
    _multiapp_level(
        isParamValid("_multiapp_level") ? parameters.get<unsigned int>("_multiapp_level") : 0),
    _multiapp_number(
        isParamValid("_multiapp_number") ? parameters.get<unsigned int>("_multiapp_number") : 0),
    _setup_timer(_perf_graph.registerSection("MooseApp::setup", 2)),
    _setup_options_timer(_perf_graph.registerSection("MooseApp::setupOptions", 5)),
    _run_input_file_timer(_perf_graph.registerSection("MooseApp::runInputFile", 3)),
    _execute_timer(_perf_graph.registerSection("MooseApp::execute", 2)),
    _execute_executioner_timer(_perf_graph.registerSection("MooseApp::executeExecutioner", 3)),
    _restore_timer(_perf_graph.registerSection("MooseApp::restore", 2)),
    _run_timer(_perf_graph.registerSection("MooseApp::run", 3)),
    _execute_mesh_modifiers_timer(_perf_graph.registerSection("MooseApp::executeMeshModifiers", 1)),
    _restore_cached_backup_timer(_perf_graph.registerSection("MooseApp::restoreCachedBackup", 2)),
    _create_minimal_app_timer(_perf_graph.registerSection("MooseApp::createMinimalApp", 3))
{
  Registry::addKnownLabel(_type);
  Moose::registerAll(_factory, _action_factory, _syntax);

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

  if (isParamValid("start_in_debugger"))
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

    std::system(command_string.c_str());

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
}

void
MooseApp::checkRegistryLabels()
{
  Registry::checkLabels();
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
        _recover_base = recover_following_arg;
    }

    // Optionally get command line argument following --recoversuffix
    // on command line.  Currently this argument applies to both
    // recovery and restart files.
    if (isParamValid("recoversuffix"))
    {
      _recover_suffix = getParam<std::string>("recoversuffix");
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
  }
  else if (getParam<bool>("apptype"))
  {
    Moose::perf_log.disable_logging();
    Moose::out << "MooseApp Type: " << type() << std::endl;
    _ready_to_exit = true;
  }
  else
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
MooseApp::setInputFileName(std::string input_filename)
{
  _input_filename = input_filename;
}

std::string
MooseApp::getOutputFileBase() const
{
  return _output_file_base;
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
MooseApp::hasRecoverFileBase()
{
  return !_recover_base.empty();
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
  TIME_SECTION(_restore_timer);

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
MooseApp::setOutputPosition(Point p)
{
  _output_position_set = true;
  _output_position = p;
  _output_warehouse.meshChanged();

  if (_executioner.get() != NULL)
    _executioner->parentOutputPositionChanged();
}

std::list<std::string>
MooseApp::getCheckpointDirectories() const
{
  // Storage for the directory names
  std::list<std::string> checkpoint_dirs;

  // Extract the CommonOutputAction
  const auto & common_actions = _action_warehouse.getActionListByName("common_output");
  mooseAssert(common_actions.size() == 1, "Should be only one common_output Action");

  const Action * common = *common_actions.begin();

  // If file_base is set in CommonOutputAction, add this file to the list of potential checkpoint
  // files
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
    MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    if (!moose_object_action)
      continue;

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

  return checkpoint_dirs;
}

std::list<std::string>
MooseApp::getCheckpointFiles() const
{
  auto checkpoint_dirs = getCheckpointDirectories();
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

void
MooseApp::registerRestartableData(std::string name,
                                  std::unique_ptr<RestartableDataValue> data,
                                  THREAD_ID tid)
{
  auto & restartable_data = _restartable_data[tid];
  auto insert_pair = moose_try_emplace(restartable_data, name, std::move(data));

  if (!insert_pair.second)
    mooseError("Attempted to declare restartable twice with the same name: ", name);
}

void
MooseApp::dynamicAppRegistration(const std::string & app_name,
                                 std::string library_path,
                                 const std::string & library_name)
{
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
}

void
MooseApp::dynamicAllRegistration(const std::string & app_name,
                                 Factory * factory,
                                 ActionFactory * action_factory,
                                 Syntax * syntax,
                                 std::string library_path,
                                 const std::string & library_name)
{
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

#ifdef LIBMESH_HAVE_DLOPEN
    void * handle = dlopen(dl_lib_full_path.c_str(), RTLD_LAZY);
#else
    void * handle = NULL;
#endif

    if (!handle)
      mooseError("Cannot open library: ", dl_lib_full_path.c_str(), "\n");

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
       * Set preparation flag after modifers are run. The final preparation
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

  // SetupMeshAction (setup_mesh)
  {
    // Build the Action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");
    action_params.set<std::string>("type") = "GeneratedMesh";
    action_params.set<std::string>("task") = "setup_mesh";

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

  // SetupMeshAction (init_mesh)
  {
    // Action parameters
    InputParameters action_params = _action_factory.getValidParams("SetupMeshAction");
    action_params.set<std::string>("type") = "GeneratedMesh";
    action_params.set<std::string>("task") = "init_mesh";

    // Build the action
    std::shared_ptr<Action> action =
        _action_factory.create("SetupMeshAction", "Mesh", action_params);
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
    // to assign a name and an ID. However, the ExecFlagTypes created by users are global constants
    // and the ID to be assigned can't be known at construction time of this global constant, it is
    // only known when it is added to this object (ExecFlagEnum). Therefore, this const cast allows
    // the ID to be set after construction. This was the lesser of two evils: const_cast or
    // friend class with mutable members.
    ExecFlagType & non_const_flag = const_cast<ExecFlagType &>(flag);
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
  bool add = true;
  for (const auto & rm : _relationship_managers)
    if (*rm == *relationship_manager)
    {
      add = false;
      break;
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
    rm->attachRelationshipManagers(rm_type);
}

std::vector<std::pair<std::string, std::string>>
MooseApp::getRelationshipManagerInfo() const
{
  std::vector<std::pair<std::string, std::string>> info_strings;
  info_strings.reserve(_relationship_managers.size());

  for (const auto & rm : _relationship_managers)
  {
    auto info = rm->getInfo();
    if (info.size())
      info_strings.emplace_back(std::make_pair(Moose::stringify(rm->getType()), info));
  }

  return info_strings;
}
