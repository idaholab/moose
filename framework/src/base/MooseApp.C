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
#include "gperftools/heap-profiler.h"
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
#include "Executor.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "CommandLine.h"
#include "InfixIterator.h"
#include "MultiApp.h"
#include "MooseUtils.h"
#include "MooseObjectAction.h"
#include "InputParameterWarehouse.h"
#include "SystemInfo.h"
#include "MooseMesh.h"
#include "FileOutput.h"
#include "ConsoleUtils.h"
#include "JsonSyntaxTree.h"
#include "JsonInputFileFormatter.h"
#include "SONDefinitionFormatter.h"
#include "RelationshipManager.h"
#include "ProxyRelationshipManager.h"
#include "Registry.h"
#include "SerializerGuard.h"
#include "PerfGraphInterface.h" // For TIME_SECTION
#include "SolutionInvalidInterface.h"
#include "Attributes.h"
#include "MooseApp.h"
#include "CommonOutputAction.h"
#include "CastUniquePointer.h"
#include "NullExecutor.h"
#include "ExecFlagRegistry.h"
#include "SolutionInvalidity.h"
#include "MooseServer.h"
#include "RestartableDataWriter.h"
#include "StringInputStream.h"
#include "MooseMain.h"

// Regular expression includes
#include "pcrecpp.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/checkpoint_io.h"
#include "libmesh/mesh_base.h"
#include "libmesh/petsc_solver_exception.h"

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
#include <filesystem>

using namespace libMesh;

#define QUOTE(macro) stringifyName(macro)

void
MooseApp::addAppParam(InputParameters & params)
{
  params.addCommandLineParam<std::string>(
      "app_to_run", "--app <type>", "Specify the application type to run (case-sensitive)");
}

void
MooseApp::addInputParam(InputParameters & params)
{
  params.addCommandLineParam<std::vector<std::string>>(
      "input_file", "-i <input file(s)>", "Specify input file(s); multiple files are merged");
}

InputParameters
MooseApp::validParams()
{
  InputParameters params = emptyInputParameters();

  MooseApp::addAppParam(params);
  MooseApp::addInputParam(params);

  params.addCommandLineParam<bool>("display_version", "-v --version", "Print application version");

  params.addOptionalValuedCommandLineParam<std::string>(
      "mesh_only",
      "--mesh-only <optional path>",
      "",
      "Build and output the mesh only (Default: \"<input_file_name>_in.e\")");

  params.addCommandLineParam<bool>(
      "show_input", "--show-input", "Shows the parsed input file before running the simulation");
  params.setGlobalCommandLineParam("show_input");
  params.addCommandLineParam<bool>(
      "show_outputs", "--show-outputs", "Shows the output execution time information");
  params.setGlobalCommandLineParam("show_outputs");
  params.addCommandLineParam<bool>(
      "show_controls", "--show-controls", "Shows the Control logic available and executed");
  params.setGlobalCommandLineParam("show_controls");

  params.addCommandLineParam<bool>(
      "no_color", "--no-color", "Disable coloring of all Console outputs");
  params.setGlobalCommandLineParam("no_color");

  MooseEnum colors("auto on off", "on");
  params.addCommandLineParam<MooseEnum>(
      "color", "--color <auto,on,off=on>", colors, "Whether to use color in console output");
  params.setGlobalCommandLineParam("color");

  params.addCommandLineParam<bool>("help", "-h --help", "Displays CLI usage statement");
  params.addCommandLineParam<bool>(
      "minimal",
      "--minimal",
      "Ignore input file and build a minimal application with Transient executioner");

  params.addCommandLineParam<bool>(
      "language_server",
      "--language-server",
      "Starts a process to communicate with development tools using the language server protocol");

  params.addCommandLineParam<bool>(
      "definition", "--definition", "Shows a SON style input definition dump for input validation");
  params.addCommandLineParam<bool>("dump", "--dump", "Shows a dump of available input file syntax");
  params.addCommandLineParam<std::string>(
      "dump_search",
      "--dump-search <search>",
      "Shows a dump of available input syntax matching a search");
  params.addCommandLineParam<bool>("registry", "--registry", "Lists all known objects and actions");
  params.addCommandLineParam<bool>(
      "registry_hit", "--registry-hit", "Lists all known objects and actions in hit format");
  params.addCommandLineParam<bool>(
      "use_executor", "--executor", "Use the new Executor system instead of Executioners");

  params.addCommandLineParam<bool>(
      "show_type", "--show-type", "Return the name of the application object");
  params.addCommandLineParam<bool>("yaml", "--yaml", "Dumps all input file syntax in YAML format");
  params.addCommandLineParam<std::string>(
      "yaml_search", "--yaml-search", "Dumps input file syntax matching a search in YAML format");
  params.addCommandLineParam<bool>("json", "--json", "Dumps all input file syntax in JSON format");
  params.addCommandLineParam<std::string>(
      "json_search", "--json-search", "Dumps input file syntax matching a search in JSON format");
  params.addCommandLineParam<bool>(
      "syntax", "--syntax", "Dumps the associated Action syntax paths ONLY");
  params.addCommandLineParam<bool>(
      "show_docs", "--docs", "Print url/path to the documentation website");
  params.addCommandLineParam<bool>("check_input",
                                   "--check-input",
                                   "Check the input file (i.e. requires -i <filename>) and quit");
  params.setGlobalCommandLineParam("check_input");
  params.addCommandLineParam<bool>(
      "show_inputs",
      "--show-copyable-inputs",
      "Shows the directories able to be copied into a user-writable location");

  params.addCommandLineParam<std::string>(
      "copy_inputs",
      "--copy-inputs <dir>",
      "Copies installed inputs (e.g. tests, examples, etc.) to a directory <appname>_<dir>");
  // TODO: Should this remain a bool? It can't be a regular argument because it contains
  // values that have dashes in it, so it'll get treated as another arg
  params.addOptionalValuedCommandLineParam<std::string>(
      "run",
      "--run <test harness args>",
      "",
      "Runs the inputs in the current directory copied to a "
      "user-writable location by \"--copy-inputs\"");

  params.addCommandLineParam<bool>(
      "list_constructed_objects",
      "--list-constructed-objects",
      "List all moose object type names constructed by the master app factory");

  params.addCommandLineParam<unsigned int>(
      "n_threads", "--n-threads=<n>", "Runs the specified number of threads per process");
  // This probably shouldn't be global, but the implications of removing this are currently
  // unknown and we need to manage it with libmesh better
  params.setGlobalCommandLineParam("n_threads");

  params.addCommandLineParam<bool>("allow_unused",
                                   "-w --allow-unused",
                                   "Warn about unused input file options instead of erroring");
  params.setGlobalCommandLineParam("allow_unused");
  params.addCommandLineParam<bool>(
      "error_unused", "-e --error-unused", "Error when encountering unused input file options");
  params.setGlobalCommandLineParam("error_unused");
  params.addCommandLineParam<bool>(
      "error_override",
      "-o --error-override",
      "Error when encountering overridden or parameters supplied multiple times");
  params.setGlobalCommandLineParam("error_override");
  params.addCommandLineParam<bool>(
      "error_deprecated", "--error-deprecated", "Turn deprecated code messages into Errors");
  params.setGlobalCommandLineParam("error_deprecated");

  params.addCommandLineParam<bool>("distributed_mesh",
                                   "--distributed-mesh",
                                   "Forces the use of a distributed finite element mesh");
  // Would prefer that this parameter isn't global, but we rely on it too much
  // in tests to be able to go back on that decision now
  params.setGlobalCommandLineParam("distributed_mesh");

  params.addCommandLineParam<std::string>(
      "split_mesh",
      "--split-mesh <splits>",
      "Comma-separated list of numbers of chunks to split the mesh into");

  // TODO: remove the logic now that this is global
  params.addCommandLineParam<std::string>(
      "split_file", "--split-file <filename>", "Name of split mesh file(s) to write/read");

  params.addCommandLineParam<bool>("use_split", "--use-split", "Use split distributed mesh files");

  params.addCommandLineParam<unsigned int>(
      "refinements", "-r <num refinements>", "Specify additional initial uniform mesh refinements");

  params.addOptionalValuedCommandLineParam<std::string>(
      "recover",
      "--recover <optional file base>",
      "",
      "Continue the calculation. Without <file base>, the most recent recovery file will be used");
  params.setGlobalCommandLineParam("recover");

  params.addCommandLineParam<bool>(
      "test_checkpoint_half_transient",
      "--test-checkpoint-half-transient",
      "Run half of a transient with checkpoints enabled; used by the TestHarness");
  params.setGlobalCommandLineParam("test_checkpoint_half_transient");

  params.addCommandLineParam<bool>(
      "trap_fpe",
      "--trap-fpe",
      "Enable floating point exception handling in critical sections of code"
#ifdef DEBUG
      " (automatic due to debug build)"
#endif
  );
  params.setGlobalCommandLineParam("trap_fpe");

  params.addCommandLineParam<bool>(
      "no_trap_fpe",
      "--no-trap-fpe",
      "Disable floating point exception handling in critical sections of code"
#ifndef DEBUG
      " (unused due to non-debug build)"
#endif
  );

  params.setGlobalCommandLineParam("no_trap_fpe");

  params.addCommandLineParam<bool>(
      "no_gdb_backtrace", "--no-gdb-backtrace", "Disables gdb backtraces.");
  params.setGlobalCommandLineParam("no_gdb_backtrace");

  params.addCommandLineParam<bool>("error", "--error", "Turn all warnings into errors");
  params.setGlobalCommandLineParam("error");

  params.addCommandLineParam<bool>("timing",
                                   "-t --timing",
                                   "Enable all performance logging for timing; disables screen "
                                   "output of performance logs for all Console objects");
  params.setGlobalCommandLineParam("timing");
  params.addCommandLineParam<bool>(
      "no_timing", "--no-timing", "Disabled performance logging; overrides -t or --timing");
  params.setGlobalCommandLineParam("no_timing");

  params.addCommandLineParam<bool>(
      "allow_test_objects", "--allow-test-objects", "Register test objects and syntax");
  params.setGlobalCommandLineParam("allow_test_objects");

  // Options ignored by MOOSE but picked up by libMesh, these are here so that they are displayed in
  // the application help
  params.addCommandLineParam<bool>(
      "keep_cout",
      "--keep-cout",
      "Keep standard output from all processors when running in parallel");
  params.setGlobalCommandLineParam("keep_cout");
  params.addCommandLineParam<bool>(
      "redirect_stdout",
      "--redirect-stdout",
      "Keep standard output from all processors when running in parallel");
  params.setGlobalCommandLineParam("redirect_stdout");

  params.addCommandLineParam<std::string>(
      "timpi_sync",
      "--timpi-sync <type=nbx>",
      "nbx",
      "Changes the sync type used in spare parallel communitations within TIMPI");
  params.setGlobalCommandLineParam("timpi_sync");

  // Options for debugging
  params.addCommandLineParam<std::string>("start_in_debugger",
                                          "--start-in-debugger <debugger>",
                                          "Start the application and attach a debugger; this will "
                                          "launch xterm windows using <debugger>");

  params.addCommandLineParam<unsigned int>(
      "stop_for_debugger",
      "--stop-for-debugger <seconds>",
      "Pauses the application during startup for <seconds> to allow for connection of debuggers");

  params.addCommandLineParam<bool>(
      "perf_graph_live_all", "--perf-graph-live-all", "Forces printing of ALL progress messages");
  params.setGlobalCommandLineParam("perf_graph_live_all");

  params.addCommandLineParam<bool>(
      "disable_perf_graph_live", "--disable-perf-graph-live", "Disables PerfGraph live printing");
  params.setGlobalCommandLineParam("disable_perf_graph_live");

  params.addParam<bool>(
      "automatic_automatic_scaling", false, "Whether to turn on automatic scaling by default");

  MooseEnum libtorch_device_type("cpu cuda mps", "cpu");
  params.addCommandLineParam<MooseEnum>("libtorch_device",
                                        "--libtorch-device",
                                        libtorch_device_type,
                                        "The device type we want to run libtorch on.");

#ifdef HAVE_GPERFTOOLS
  params.addCommandLineParam<std::string>(
      "gperf_profiler_on",
      "--gperf-profiler-on <ranks>",
      "To generate profiling report only on comma-separated list of MPI ranks");
#endif

  params.addCommandLineParam<bool>(
      "show_data_params",
      "--show-data-params",
      false,
      "Show found paths for all DataFileName parameters in the header");
  params.addCommandLineParam<bool>("show_data_paths",
                                   "--show-data-paths",
                                   false,
                                   "Show registered data paths for searching in the header");

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
  params.addPrivateParam<std::unique_ptr<Backup> *>("_initial_backup", nullptr);
  params.addPrivateParam<std::shared_ptr<Parser>>("_parser");

  params.addParam<bool>(
      "use_legacy_material_output",
      true,
      "Set false to allow material properties to be output on INITIAL, not just TIMESTEP_END.");
  params.addParam<bool>(
      "use_legacy_initial_residual_evaluation_behavior",
      true,
      "The legacy behavior performs an often times redundant residual evaluation before the "
      "solution modifying objects are executed prior to the initial (0th nonlinear iteration) "
      "residual evaluation. The new behavior skips that redundant residual evaluation unless the "
      "parameter Executioner/use_pre_SMO_residual is set to true.");

  params.addParam<bool>(
      MeshGeneratorSystem::allow_data_driven_param,
      false,
      "Set true to enable data-driven mesh generation, which is an experimental feature");

  MooseApp::addAppParam(params);

  return params;
}

MooseApp::MooseApp(InputParameters parameters)
  : ConsoleStreamInterface(*this),
    PerfGraphInterface(*this, "MooseApp"),
    ParallelObject(*parameters.get<std::shared_ptr<Parallel::Communicator>>(
        "_comm")), // Can't call getParam() before pars is set
    MooseBase(parameters.get<std::string>("_type"),
              parameters.get<std::string>("_app_name"),
              *this,
              _pars),
    _pars(parameters),
    _comm(getParam<std::shared_ptr<Parallel::Communicator>>("_comm")),
    _file_base_set_by_user(false),
    _output_position_set(false),
    _start_time_set(false),
    _start_time(0.0),
    _global_time_offset(0.0),
    _input_parameter_warehouse(std::make_unique<InputParameterWarehouse>()),
    _action_factory(*this),
    _action_warehouse(*this, _syntax, _action_factory),
    _output_warehouse(*this),
    _parser(parameters.get<std::shared_ptr<Parser>>("_parser")),
    _builder(*this, _action_warehouse, _parser),
    _restartable_data(libMesh::n_threads()),
    _perf_graph(createRecoverablePerfGraph()),
    _solution_invalidity(createRecoverableSolutionInvalidity()),
    _rank_map(*_comm, _perf_graph),
    _use_executor(parameters.get<bool>("use_executor")),
    _null_executor(NULL),
    _use_nonlinear(true),
    _use_eigen_value(false),
    _enable_unused_check(ERROR_UNUSED),
    _factory(*this),
    _error_overridden(false),
    _ready_to_exit(false),
    _exit_code(0),
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
    _test_checkpoint_half_transient(false),
    _check_input(getParam<bool>("check_input")),
    _multiapp_level(
        isParamValid("_multiapp_level") ? parameters.get<unsigned int>("_multiapp_level") : 0),
    _multiapp_number(
        isParamValid("_multiapp_number") ? parameters.get<unsigned int>("_multiapp_number") : 0),
    _master_mesh(isParamValid("_master_mesh") ? parameters.get<const MooseMesh *>("_master_mesh")
                                              : nullptr),
    _master_displaced_mesh(isParamValid("_master_displaced_mesh")
                               ? parameters.get<const MooseMesh *>("_master_displaced_mesh")
                               : nullptr),
    _mesh_generator_system(*this),
    _chain_control_system(*this),
    _rd_reader(*this, _restartable_data),
    _execute_flags(moose::internal::ExecFlagRegistry::getExecFlagRegistry().getFlags()),
    _output_buffer_cache(nullptr),
    _automatic_automatic_scaling(getParam<bool>("automatic_automatic_scaling")),
    _initial_backup(getParam<std::unique_ptr<Backup> *>("_initial_backup"))
#ifdef LIBTORCH_ENABLED
    ,
    _libtorch_device(determineLibtorchDeviceType(getParam<MooseEnum>("libtorch_device")))
#endif
{
  // Set the TIMPI sync type via --timpi-sync
  const auto & timpi_sync = parameters.get<std::string>("timpi_sync");
  const_cast<Parallel::Communicator &>(comm()).sync_type(timpi_sync);

#ifdef HAVE_GPERFTOOLS
  if (isUltimateMaster())
  {
    bool has_cpu_profiling = false;
    bool has_heap_profiling = false;
    static std::string cpu_profile_file;
    static std::string heap_profile_file;

    // For CPU profiling, users need to have environment 'MOOSE_PROFILE_BASE'
    if (std::getenv("MOOSE_PROFILE_BASE"))
    {
      has_cpu_profiling = true;
      cpu_profile_file =
          std::getenv("MOOSE_PROFILE_BASE") + std::to_string(_comm->rank()) + ".prof";
      // create directory if needed
      auto name = MooseUtils::splitFileName(cpu_profile_file);
      if (!name.first.empty())
      {
        if (processor_id() == 0)
          MooseUtils::makedirs(name.first.c_str());
        _comm->barrier();
      }
    }

    // For Heap profiling, users need to have 'MOOSE_HEAP_BASE'
    if (std::getenv("MOOSE_HEAP_BASE"))
    {
      has_heap_profiling = true;
      heap_profile_file = std::getenv("MOOSE_HEAP_BASE") + std::to_string(_comm->rank());
      // create directory if needed
      auto name = MooseUtils::splitFileName(heap_profile_file);
      if (!name.first.empty())
      {
        if (processor_id() == 0)
          MooseUtils::makedirs(name.first.c_str());
        _comm->barrier();
      }
    }

    // turn on profiling only on selected ranks
    if (isParamSetByUser("gperf_profiler_on"))
    {
      auto rankstr = getParam<std::string>("gperf_profiler_on");
      std::vector<processor_id_type> ranks;
      bool success = MooseUtils::tokenizeAndConvert(rankstr, ranks, ", ");
      if (!success)
        mooseError("Invalid argument for --gperf-profiler-on: '", rankstr, "'");
      for (auto & rank : ranks)
      {
        if (rank >= _comm->size())
          mooseError("Invalid argument for --gperf-profiler-on: ",
                     rank,
                     " is greater than or equal to ",
                     _comm->size());
        if (rank == _comm->rank())
        {
          _cpu_profiling = has_cpu_profiling;
          _heap_profiling = has_heap_profiling;
        }
      }
    }
    else
    {
      _cpu_profiling = has_cpu_profiling;
      _heap_profiling = has_heap_profiling;
    }

    if (_cpu_profiling)
      if (!ProfilerStart(cpu_profile_file.c_str()))
        mooseError("CPU profiler is not started properly");

    if (_heap_profiling)
    {
      HeapProfilerStart(heap_profile_file.c_str());
      if (!IsHeapProfilerRunning())
        mooseError("Heap profiler is not started properly");
    }
  }
#else
  if (std::getenv("MOOSE_PROFILE_BASE") || std::getenv("MOOSE_HEAP_BASE"))
    mooseError("gperftool is not available for CPU or heap profiling");
#endif

  // If this will be a language server then turn off output until that starts
  if (isParamValid("language_server") && getParam<bool>("language_server"))
    _output_buffer_cache = Moose::out.rdbuf(nullptr);

  Registry::addKnownLabel(_type);
  Moose::registerAll(_factory, _action_factory, _syntax);

  _the_warehouse = std::make_unique<TheWarehouse>();
  _the_warehouse->registerAttribute<AttribMatrixTags>("matrix_tags", 0);
  _the_warehouse->registerAttribute<AttribVectorTags>("vector_tags", 0);
  _the_warehouse->registerAttribute<AttribExecOns>("exec_ons", 0);
  _the_warehouse->registerAttribute<AttribSubdomains>("subdomains", 0);
  _the_warehouse->registerAttribute<AttribBoundaries>("boundaries", 0);
  _the_warehouse->registerAttribute<AttribThread>("thread", 0);
  _the_warehouse->registerAttribute<AttribExecutionOrderGroup>("execution_order_group", 0);
  _the_warehouse->registerAttribute<AttribPreIC>("pre_ic", 0);
  _the_warehouse->registerAttribute<AttribPreAux>("pre_aux");
  _the_warehouse->registerAttribute<AttribPostAux>("post_aux");
  _the_warehouse->registerAttribute<AttribName>("name", "dummy");
  _the_warehouse->registerAttribute<AttribSystem>("system", "dummy");
  _the_warehouse->registerAttribute<AttribVar>("variable", -1);
  _the_warehouse->registerAttribute<AttribInterfaces>("interfaces", 0);
  _the_warehouse->registerAttribute<AttribSysNum>("sys_num", libMesh::invalid_uint);
  _the_warehouse->registerAttribute<AttribResidualObject>("residual_object");
  _the_warehouse->registerAttribute<AttribSorted>("sorted");
  _the_warehouse->registerAttribute<AttribDisplaced>("displaced", -1);

  _perf_graph.enableLivePrint();

  if (isParamValid("_argc") && isParamValid("_argv"))
  {
    int argc = getParam<int>("_argc");
    char ** argv = getParam<char **>("_argv");

    _sys_info = std::make_unique<SystemInfo>(argc, argv);
  }
  if (isParamValid("_command_line"))
    _command_line = getParam<std::shared_ptr<CommandLine>>("_command_line");
  else
    mooseError("Valid CommandLine object required");

  if (_check_input && isParamSetByUser("recover"))
    mooseError("Cannot run --check-input with --recover. Recover files might not exist");

  if (isParamSetByUser("start_in_debugger") && isUltimateMaster())
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

  if (isParamSetByUser("stop_for_debugger") && isUltimateMaster())
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
  registerRestartableDataMapName(MooseApp::MESH_META_DATA, MooseApp::MESH_META_DATA_SUFFIX);

  if (parameters.have_parameter<bool>("use_legacy_dirichlet_bc"))
    mooseDeprecated("The parameter 'use_legacy_dirichlet_bc' is no longer valid.\n\n",
                    "All Dirichlet boundary conditions are preset by default.\n\n",
                    "Remove said parameter in ",
                    name(),
                    " to remove this deprecation warning.");

  Moose::out << std::flush;
}

MooseApp::~MooseApp()
{
#ifdef HAVE_GPERFTOOLS
  // CPU profiling stop
  if (_cpu_profiling)
    ProfilerStop();
  // Heap profiling stop
  if (_heap_profiling)
    HeapProfilerStop();
#endif
  _action_warehouse.clear();
  _executioner.reset();
  _the_warehouse.reset();

  // Don't wait for implicit destruction of input parameter storage
  _input_parameter_warehouse.reset();

  // This is dirty, but I don't know what else to do. Obviously, others
  // have had similar problems if you look above. In specific, the
  // dlclose below on macs is destructing some data that does not
  // belong to it in garbage collection. So... don't even give
  // dlclose an option
  _restartable_data.clear();

#ifdef LIBMESH_HAVE_DLOPEN
  // Close any open dynamic libraries
  for (const auto & lib_pair : _lib_handles)
    dlclose(lib_pair.second.library_handle);
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
  TIME_SECTION("setupOptions", 5, "Setting Up Options");

  // Print the header, this is as early as possible
  auto hdr = header();
  if (hdr.length() != 0)
  {
    if (multiAppLevel() > 0)
      MooseUtils::indentMessage(_name, hdr);
    Moose::out << hdr << std::endl;
  }

  if (getParam<bool>("error_unused"))
    setCheckUnusedFlag(true);
  else if (getParam<bool>("allow_unused"))
    setCheckUnusedFlag(false);

  if (getParam<bool>("error_override"))
    setErrorOverridden();

  _distributed_mesh_on_command_line = getParam<bool>("distributed_mesh");

  _test_checkpoint_half_transient = getParam<bool>("test_checkpoint_half_transient");

  // The no_timing flag takes precedence over the timing flag.
  if (getParam<bool>("no_timing"))
    _pars.set<bool>("timing") = false;

  if (getParam<bool>("trap_fpe"))
  {
    _trap_fpe = true;
    _perf_graph.setActive(false);
    if (getParam<bool>("no_trap_fpe"))
      mooseError("Cannot use both \"--trap-fpe\" and \"--no-trap-fpe\" flags.");
  }
  else if (getParam<bool>("no_trap_fpe"))
    _trap_fpe = false;

  // Turn all warnings in MOOSE to errors (almost see next logic block)
  Moose::_warnings_are_errors = getParam<bool>("error");

  // Deprecated messages can be toggled to errors independently from everything else.
  Moose::_deprecated_is_error = getParam<bool>("error_deprecated");

  if (isUltimateMaster()) // makes sure coloring isn't reset incorrectly in multi-app settings
  {
    // Set from command line
    auto color = getParam<MooseEnum>("color");
    // Set from environment
    char * c_color = std::getenv("MOOSE_COLOR");
    if (c_color)
      color.assign(std::string(c_color), "While assigning environment variable MOOSE_COLOR");
    // Set from deprecated --no-color
    if (getParam<bool>("no_color"))
      color = "off";

    if (color == "auto")
      Moose::setColorConsole(true);
    else if (color == "on")
      Moose::setColorConsole(true, true);
    else if (color == "off")
      Moose::setColorConsole(false);
    else
      mooseAssert(false, "Should not hit");

    // After setting color so that non-yellow deprecated is honored
    if (getParam<bool>("no_color"))
      mooseDeprecated("The --no-color flag is deprecated. Use '--color off' instead.");
  }

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
    Moose::out << getPrintableVersion() << std::endl;
    _ready_to_exit = true;
    return;
  }
  else if (getParam<bool>("help"))
  {
    _command_line->printUsage();
    _ready_to_exit = true;
  }
  else if (getParam<bool>("dump") || isParamSetByUser("dump_search"))
  {
    const std::string search =
        isParamSetByUser("dump_search") ? getParam<std::string>("dump_search") : "";

    JsonSyntaxTree tree(search);

    {
      TIME_SECTION("dump", 1, "Building Syntax Tree");
      _builder.buildJsonSyntaxTree(tree);
    }

    // Check if second arg is valid or not
    if ((tree.getRoot()).is_object())
    {
      // Turn off live printing so that it doesn't mess with the dump
      _perf_graph.disableLivePrint();

      JsonInputFileFormatter formatter;
      Moose::out << "\n### START DUMP DATA ###\n"
                 << formatter.toString(tree.getRoot()) << "\n### END DUMP DATA ###" << std::endl;
      _ready_to_exit = true;
    }
    else
      mooseError("Search parameter '", search, "' was not found in the registered syntax.");
  }
  else if (getParam<bool>("registry"))
  {
    _perf_graph.disableLivePrint();

    Moose::out << "Label\tType\tName\tClass\tFile\n";

    auto & objmap = Registry::allObjects();
    for (auto & entry : objmap)
      for (auto & obj : entry.second)
        Moose::out << entry.first << "\tobject\t" << obj->name() << "\t" << obj->_classname << "\t"
                   << obj->_file << "\n";

    auto & actmap = Registry::allActions();
    for (auto & entry : actmap)
    {
      for (auto & act : entry.second)
        Moose::out << entry.first << "\taction\t" << act->_name << "\t" << act->_classname << "\t"
                   << act->_file << "\n";
    }

    _ready_to_exit = true;
  }
  else if (getParam<bool>("registry_hit"))
  {
    _perf_graph.disableLivePrint();

    Moose::out << "### START REGISTRY DATA ###\n";

    hit::Section root("");
    auto sec = new hit::Section("registry");
    root.addChild(sec);
    auto objsec = new hit::Section("objects");
    sec->addChild(objsec);

    auto & objmap = Registry::allObjects();
    for (auto & entry : objmap)
      for (auto & obj : entry.second)
      {
        auto ent = new hit::Section("entry");
        objsec->addChild(ent);
        ent->addChild(new hit::Field("label", hit::Field::Kind::String, entry.first));
        ent->addChild(new hit::Field("type", hit::Field::Kind::String, "object"));
        ent->addChild(new hit::Field("name", hit::Field::Kind::String, obj->name()));
        ent->addChild(new hit::Field("class", hit::Field::Kind::String, obj->_classname));
        ent->addChild(new hit::Field("file", hit::Field::Kind::String, obj->_file));
      }

    auto actsec = new hit::Section("actions");
    sec->addChild(actsec);
    auto & actmap = Registry::allActions();
    for (auto & entry : actmap)
      for (auto & act : entry.second)
      {
        auto ent = new hit::Section("entry");
        actsec->addChild(ent);
        ent->addChild(new hit::Field("label", hit::Field::Kind::String, entry.first));
        ent->addChild(new hit::Field("type", hit::Field::Kind::String, "action"));
        ent->addChild(new hit::Field("task", hit::Field::Kind::String, act->_name));
        ent->addChild(new hit::Field("class", hit::Field::Kind::String, act->_classname));
        ent->addChild(new hit::Field("file", hit::Field::Kind::String, act->_file));
      }

    Moose::out << root.render();

    Moose::out << "\n### END REGISTRY DATA ###\n";
    _ready_to_exit = true;
  }
  else if (getParam<bool>("definition"))
  {
    _perf_graph.disableLivePrint();

    JsonSyntaxTree tree("");
    _builder.buildJsonSyntaxTree(tree);
    SONDefinitionFormatter formatter;
    Moose::out << "%-START-SON-DEFINITION-%\n"
               << formatter.toString(tree.getRoot()) << "\n%-END-SON-DEFINITION-%\n";
    _ready_to_exit = true;
  }
  else if (getParam<bool>("yaml") || isParamSetByUser("yaml_search"))
  {
    const std::string search =
        isParamSetByUser("yaml_search") ? getParam<std::string>("yaml_search") : "";
    _perf_graph.disableLivePrint();

    _builder.initSyntaxFormatter(Moose::Builder::YAML, true);
    _builder.buildFullTree(search);

    _ready_to_exit = true;
  }
  else if (getParam<bool>("json") || isParamSetByUser("json_search"))
  {
    const std::string search =
        isParamSetByUser("json_search") ? getParam<std::string>("json_search") : "";
    _perf_graph.disableLivePrint();

    JsonSyntaxTree tree(search);
    _builder.buildJsonSyntaxTree(tree);

    Moose::out << "**START JSON DATA**\n" << tree.getRoot().dump(2) << "\n**END JSON DATA**\n";
    _ready_to_exit = true;
  }
  else if (getParam<bool>("syntax"))
  {
    _perf_graph.disableLivePrint();

    std::multimap<std::string, Syntax::ActionInfo> syntax = _syntax.getAssociatedActions();
    Moose::out << "**START SYNTAX DATA**\n";
    for (const auto & it : syntax)
      Moose::out << it.first << "\n";
    Moose::out << "**END SYNTAX DATA**\n" << std::endl;
    _ready_to_exit = true;
  }
  else if (getParam<bool>("show_type"))
  {
    _perf_graph.disableLivePrint();

    Moose::out << "MooseApp Type: " << type() << std::endl;
    _ready_to_exit = true;
  }
  else if (getInputFileNames().size())
  {
    if (isParamSetByUser("recover"))
    {
      // We need to set the flag manually here since the recover parameter is a string type (takes
      // an optional filename)
      _recover = true;
      const auto & recover = getParam<std::string>("recover");
      if (recover.size())
        _restart_recover_base = recover;
    }

    // In the event that we've parsed once before already in MooseMain, we
    // won't need to parse again
    if (!_parser->root())
      _parser->parse();

    _builder.build();

    if (isParamSetByUser("mesh_only"))
    {
      _syntax.registerTaskName("mesh_only", true);
      _syntax.addDependency("mesh_only", "setup_mesh_complete");
      _syntax.addDependency("determine_system_type", "mesh_only");
      _action_warehouse.setFinalTask("mesh_only");
    }
    else if (isParamSetByUser("split_mesh"))
    {
      _split_mesh = true;
      _syntax.registerTaskName("split_mesh", true);
      _syntax.addDependency("split_mesh", "setup_mesh_complete");
      _syntax.addDependency("determine_system_type", "split_mesh");
      _action_warehouse.setFinalTask("split_mesh");
    }
    _action_warehouse.build();

    // Setup the AppFileBase for use by the Outputs or other systems that need output file info
    {
      // Extract the CommonOutputAction
      const auto common_actions = _action_warehouse.getActions<CommonOutputAction>();
      mooseAssert(common_actions.size() <= 1, "Should not be more than one CommonOutputAction");
      const Action * common = common_actions.empty() ? nullptr : *common_actions.begin();

      // If file_base is set in CommonOutputAction through parsing input, obtain the file_base
      if (common && common->isParamValid("file_base"))
      {
        _output_file_base = common->getParam<std::string>("file_base");
        _file_base_set_by_user = true;
      }
      else if (isUltimateMaster())
      {
        // if this app is a master, we use the first input file name as the default file base.
        // use proximate here because the input file is an absolute path
        const auto & base = getLastInputFileName();
        size_t pos = base.find_last_of('.');
        _output_file_base = base.substr(0, pos);
        // Note: we did not append "_out" in the file base here because we do not want to
        //       have it in between the input file name and the object name for Output/*
        //       syntax.
      }
      // default file base for multiapps is set by MultiApp
    }
  }
  // No input file provided but we have other arguments (so don't just show print usage)
  else if (!isParamSetByUser("input_file") && _command_line->getArguments().size() > 2)
  {
    mooseAssert(getInputFileNames().empty(), "Should be empty");

    if (_check_input)
      mooseError("You specified --check-input, but did not provide an input file. Add -i "
                 "<inputfile> to your command line.");

    mooseError("No input files specified. Add -i <inputfile> to your command line.");
  }
  else if (isParamValid("language_server") && getParam<bool>("language_server"))
  {
    _perf_graph.disableLivePrint();

    // Reset output to the buffer what was cached before it was turned it off
    if (!Moose::out.rdbuf() && _output_buffer_cache)
      Moose::out.rdbuf(_output_buffer_cache);

    // Start a language server that communicates using an iostream connection
    MooseServer moose_server(*this);

    moose_server.run();

    _ready_to_exit = true;
  }

  else /* The catch-all case for bad options or missing options, etc. */
  {
    _command_line->printUsage();
    _ready_to_exit = true;
    _exit_code = 1;
  }

  Moose::out << std::flush;
}

const std::vector<std::string> &
MooseApp::getInputFileNames() const
{
  mooseAssert(_parser, "Parser is not set");
  return _parser->getInputFileNames();
}

const std::string &
MooseApp::getLastInputFileName() const
{
  mooseAssert(_parser, "Parser is not set");
  return _parser->getLastInputFileName();
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
MooseApp::setOutputFileBase(const std::string & output_file_base)
{
  _output_file_base = output_file_base;

  // Reset the file base in the outputs
  _output_warehouse.resetFileBase();

  // Reset the file base in multiapps (if they have been constructed yet)
  if (getExecutioner())
    for (auto & multi_app : feProblem().getMultiAppWarehouse().getObjects())
      multi_app->setAppOutputFileBase();

  _file_base_set_by_user = true;
}

void
MooseApp::runInputFile()
{
  TIME_SECTION("runInputFile", 3);

  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  _action_warehouse.executeAllActions();

  if (isParamSetByUser("mesh_only") || isParamSetByUser("split_mesh"))
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

  _builder.errorCheck(*_comm, warn, err);

  auto apps = feProblem().getMultiAppWarehouse().getObjects();
  for (auto app : apps)
    for (unsigned int i = 0; i < app->numLocalApps(); i++)
      app->localApp(i)->errorCheck();
}

void
MooseApp::executeExecutioner()
{
  TIME_SECTION("executeExecutioner", 3);

  // If ready to exit has been set, then just return
  if (_ready_to_exit)
    return;

  // run the simulation
  if (_use_executor && _executor)
  {
    LibmeshPetscCall(Moose::PetscSupport::petscSetupOutput(_command_line.get()));
    _executor->init();
    errorCheck();
    auto result = _executor->exec();
    if (!result.convergedAll())
      mooseError(result.str());
  }
  else if (_executioner)
  {
    LibmeshPetscCall(Moose::PetscSupport::petscSetupOutput(_command_line.get()));
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

std::vector<std::filesystem::path>
MooseApp::backup(const std::filesystem::path & folder_base)
{
  TIME_SECTION("backup", 2, "Backing Up Application to File");

  preBackup();

  RestartableDataWriter writer(*this, _restartable_data);
  return writer.write(folder_base);
}

std::unique_ptr<Backup>
MooseApp::backup()
{
  TIME_SECTION("backup", 2, "Backing Up Application");

  RestartableDataWriter writer(*this, _restartable_data);

  preBackup();

  auto backup = std::make_unique<Backup>();
  writer.write(*backup->header, *backup->data);

  return backup;
}

void
MooseApp::restore(const std::filesystem::path & folder_base, const bool for_restart)
{
  TIME_SECTION("restore", 2, "Restoring Application from File");

  const DataNames filter_names = for_restart ? getRecoverableData() : DataNames{};

  _rd_reader.setInput(folder_base);
  _rd_reader.restore(filter_names);

  postRestore(for_restart);
}

void
MooseApp::restore(std::unique_ptr<Backup> backup, const bool for_restart)
{
  TIME_SECTION("restore", 2, "Restoring Application");

  const DataNames filter_names = for_restart ? getRecoverableData() : DataNames{};

  if (!backup)
    mooseError("MooseApp::resore(): Provided backup is not initialized");

  auto header = std::move(backup->header);
  mooseAssert(header, "Header not available");

  auto data = std::move(backup->data);
  mooseAssert(data, "Data not available");

  _rd_reader.setInput(std::move(header), std::move(data));
  _rd_reader.restore(filter_names);

  postRestore(for_restart);
}

void
MooseApp::restoreFromInitialBackup(const bool for_restart)
{
  mooseAssert(hasInitialBackup(), "Missing initial backup");
  restore(std::move(*_initial_backup), for_restart);
}

std::unique_ptr<Backup>
MooseApp::finalizeRestore()
{
  if (!_rd_reader.isRestoring())
    mooseError("MooseApp::finalizeRestore(): Not currently restoring");

  // This gives us access to the underlying streams so that we can return it if needed
  auto input_streams = _rd_reader.clear();

  std::unique_ptr<Backup> backup;

  // Give them back a backup if this restore started from a Backup, in which case
  // the two streams in the Backup are formed into StringInputStreams
  if (auto header_string_input = dynamic_cast<StringInputStream *>(input_streams.header.get()))
  {
    auto data_string_input = dynamic_cast<StringInputStream *>(input_streams.data.get());
    mooseAssert(data_string_input, "Should also be a string input");

    auto header_sstream = header_string_input->release();
    mooseAssert(header_sstream, "Header not available");

    auto data_sstream = data_string_input->release();
    mooseAssert(data_sstream, "Data not available");

    backup = std::make_unique<Backup>();
    backup->header = std::move(header_sstream);
    backup->data = std::move(data_sstream);
  }

  return backup;
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

FEProblemBase &
MooseApp::feProblem() const
{
  return _executor.get() ? _executor->feProblem() : _executioner->feProblem();
}

void
MooseApp::addExecutor(const std::string & type,
                      const std::string & name,
                      const InputParameters & params)
{
  std::shared_ptr<Executor> executor = _factory.create<Executor>(type, name, params);

  if (_executors.count(executor->name()) > 0)
    mooseError("an executor with name '", executor->name(), "' already exists");
  _executors[executor->name()] = executor;
}

void
MooseApp::addExecutorParams(const std::string & type,
                            const std::string & name,
                            const InputParameters & params)
{
  _executor_params[name] = std::make_pair(type, std::make_unique<InputParameters>(params));
}

Parser &
MooseApp::parser()
{
  mooseAssert(_parser, "Not set");
  return *_parser;
}

void
MooseApp::recursivelyCreateExecutors(const std::string & current_executor_name,
                                     std::list<std::string> & possible_roots,
                                     std::list<std::string> & current_branch)
{
  // Did we already make this one?
  if (_executors.find(current_executor_name) != _executors.end())
    return;

  // Is this one already on the current branch (i.e. there is a cycle)
  if (std::find(current_branch.begin(), current_branch.end(), current_executor_name) !=
      current_branch.end())
  {
    std::stringstream exec_names_string;

    auto branch_it = current_branch.begin();

    exec_names_string << *branch_it++;

    for (; branch_it != current_branch.end(); ++branch_it)
      exec_names_string << ", " << *branch_it;

    exec_names_string << ", " << current_executor_name;

    mooseError("Executor cycle detected: ", exec_names_string.str());
  }

  current_branch.push_back(current_executor_name);

  // Build the dependencies first
  const auto & params = *_executor_params[current_executor_name].second;

  for (const auto & param : params)
  {
    if (params.have_parameter<ExecutorName>(param.first))
    {
      const auto & dependency_name = params.get<ExecutorName>(param.first);

      possible_roots.remove(dependency_name);

      if (!dependency_name.empty())
        recursivelyCreateExecutors(dependency_name, possible_roots, current_branch);
    }
  }

  // Add this Executor
  const auto & type = _executor_params[current_executor_name].first;
  addExecutor(type, current_executor_name, params);

  current_branch.pop_back();
}

void
MooseApp::createExecutors()
{
  // Do we have any?
  if (_executor_params.empty())
    return;

  // Holds the names of Executors that may be the root executor
  std::list<std::string> possibly_root;

  // What is already built
  std::map<std::string, bool> already_built;

  // The Executors that are currently candidates for being roots
  std::list<std::string> possible_roots;

  // The current line of dependencies - used for finding cycles
  std::list<std::string> current_branch;

  // Build the NullExecutor
  {
    auto params = _factory.getValidParams("NullExecutor");
    _null_executor = _factory.create<NullExecutor>("NullExecutor", "_null_executor", params);
  }

  for (const auto & params_entry : _executor_params)
  {
    const auto & name = params_entry.first;

    // Did we already make this one?
    if (_executors.find(name) != _executors.end())
      continue;

    possible_roots.emplace_back(name);

    recursivelyCreateExecutors(name, possible_roots, current_branch);
  }

  // If there is more than one possible root - error
  if (possible_roots.size() > 1)
  {
    auto root_string_it = possible_roots.begin();

    std::stringstream roots_string;

    roots_string << *root_string_it++;

    for (; root_string_it != possible_roots.end(); ++root_string_it)
      roots_string << ", " << *root_string_it;

    mooseError("Multiple Executor roots found: ", roots_string.str());
  }

  // Set the root executor
  _executor = _executors[possible_roots.front()];
}

Executor &
MooseApp::getExecutor(const std::string & name, bool fail_if_not_found)
{
  auto it = _executors.find(name);

  if (it != _executors.end())
    return *it->second;

  if (fail_if_not_found)
    mooseError("Executor not found: ", name);

  return *_null_executor;
}

Executioner *
MooseApp::getExecutioner() const
{
  return _executioner.get() ? _executioner.get() : _executor.get();
}

void
MooseApp::setErrorOverridden()
{
  _error_overridden = true;
}

void
MooseApp::run()
{
  TIME_SECTION("run", 3);
  if (getParam<bool>("show_docs"))
  {
    auto binname = appBinaryName();
    if (binname == "")
      mooseError("could not locate installed tests to run (unresolved binary/app name)");
    auto docspath = MooseUtils::docsDir(binname);
    if (docspath == "")
      mooseError("no installed documentation found");

    auto docmsgfile = MooseUtils::pathjoin(docspath, "docmsg.txt");
    std::string docmsg = "file://" + MooseUtils::realpath(docspath) + "/index.html";
    if (MooseUtils::pathExists(docmsgfile) && MooseUtils::checkFileReadable(docmsgfile))
    {
      std::ifstream ifs(docmsgfile);
      std::string content((std::istreambuf_iterator<char>(ifs)),
                          (std::istreambuf_iterator<char>()));
      content.replace(content.find("$LOCAL_SITE_HOME"), content.length(), docmsg);
      docmsg = content;
    }

    Moose::out << docmsg << "\n";
    _ready_to_exit = true;
    return;
  }

  if (showInputs() || copyInputs() || runInputs())
  {
    _ready_to_exit = true;
    return;
  }

  try
  {
    TIME_SECTION("setup", 2, "Setting Up");
    setupOptions();
    runInputFile();
  }
  catch (std::exception & err)
  {
    mooseError(err.what());
  }

  if (!_check_input)
  {
    TIME_SECTION("execute", 2, "Executing");
    executeExecutioner();
  }
  else
  {
    errorCheck();
    // Output to stderr, so it is easier for peacock to get the result
    Moose::err << "Syntax OK" << std::endl;
  }
}

bool
MooseApp::showInputs() const
{
  if (getParam<bool>("show_inputs"))
  {
    const auto show_inputs_syntax = _pars.getCommandLineMetadata("show_inputs").switches;
    std::vector<std::string> dirs;
    const auto installable_inputs = getInstallableInputs();

    if (installable_inputs == "")
    {
      Moose::out
          << "Show inputs has not been overriden in this application.\nContact the developers of "
             "this appication and request that they override \"MooseApp::getInstallableInputs\".\n";
    }
    else
    {
      mooseAssert(!show_inputs_syntax.empty(), "show_inputs sytnax should not be empty");

      MooseUtils::tokenize(installable_inputs, dirs, 1, " ");
      Moose::out << "The following directories are installable into a user-writeable directory:\n\n"
                 << installable_inputs << '\n'
                 << "\nTo install one or more directories of inputs, execute the binary with the \""
                 << show_inputs_syntax[0] << "\" flag. e.g.:\n$ "
                 << _command_line->getExecutableName() << ' ' << show_inputs_syntax[0] << ' '
                 << dirs[0] << '\n';
    }
    return true;
  }
  return false;
}

std::string
MooseApp::getInstallableInputs() const
{
  return "tests";
}

bool
MooseApp::copyInputs()
{
  if (isParamSetByUser("copy_inputs"))
  {
    if (comm().size() > 1)
      mooseError("The --copy-inputs option should not be ran in parallel");

    // Get command line argument following --copy-inputs on command line
    auto dir_to_copy = getParam<std::string>("copy_inputs");

    if (dir_to_copy.empty())
      mooseError("Error retrieving directory to copy");
    if (dir_to_copy.back() != '/')
      dir_to_copy += '/';

    // This binary name is the actual binary. That is, if we called a symlink it'll
    // be the name of what the symlink points to
    auto binname = appBinaryName();
    if (binname == "")
      mooseError("could not locate installed tests to run (unresolved binary/app name)");

    auto src_dir = MooseUtils::installedInputsDir(
        binname,
        dir_to_copy,
        "Rerun binary with " + _pars.getCommandLineMetadata("show_inputs").switches[0] +
            " to get a list of installable directories.");

    // Use the command line here because if we have a symlink to another binary,
    // we want to dump into a directory that is named after the symlink not the true binary
    auto dst_dir = _command_line->getExecutableNameBase() + "/" + dir_to_copy;
    auto cmdname = _command_line->getExecutableName();
    if (cmdname.find_first_of("/") != std::string::npos)
      cmdname = cmdname.substr(cmdname.find_first_of("/") + 1, std::string::npos);

    if (MooseUtils::pathExists(dst_dir))
      mooseError(
          "The directory \"./",
          dst_dir,
          "\" already exists.\nTo update/recopy the contents of this directory, rename (\"mv ",
          dst_dir,
          " new_dir_name\") or remove (\"rm -r ",
          dst_dir,
          "\") the existing directory.\nThen re-run \"",
          cmdname,
          " --copy-inputs ",
          dir_to_copy,
          "\".");

    std::string cmd = "mkdir -p " + dst_dir + "; rsync -av " + src_dir + " " + dst_dir;

    TIME_SECTION("copy_inputs", 2, "Copying Inputs");

    mooseAssert(comm().size() == 1, "Should be run in serial");
    const auto return_value = system(cmd.c_str());
    if (!WIFEXITED(return_value))
      mooseError("Process exited unexpectedly");
    setExitCode(WEXITSTATUS(return_value));
    if (exitCode() == 0)
      Moose::out << "Directory successfully copied into ./" << dst_dir << '\n';
    return true;
  }
  return false;
}

bool
MooseApp::runInputs()
{
  if (isParamSetByUser("run"))
  {
    if (comm().size() > 1)
      mooseError("The --run option should not be ran in parallel");

    // Pass everything after --run on the cli to the TestHarness
    const auto find_run_it = std::as_const(*_command_line).findCommandLineParam("run");
    const auto & cl_entries = std::as_const(*_command_line).getEntries();
    mooseAssert(find_run_it != cl_entries.end(), "Didn't find the option");
    std::string test_args;
    for (auto it = std::next(find_run_it); it != cl_entries.end(); ++it)
      for (const auto & arg : it->raw_args)
      {
        test_args += " " + arg;
        libMesh::add_command_line_name(arg);
      }

    auto working_dir = MooseUtils::getCurrentWorkingDir();
    if (MooseUtils::findTestRoot() == "")
    {
      auto bin_name = appBinaryName();
      if (bin_name == "")
        mooseError("Could not locate binary name relative to installed location");

      auto cmd_name = Moose::getExecutableName();
      mooseError(
          "Could not locate installed tests from the current working directory:",
          working_dir,
          ".\nMake sure you are executing this command from within a writable installed inputs ",
          "directory.\nRun \"",
          cmd_name,
          " --copy-inputs <dir>\" to copy the contents of <dir> to a \"./",
          bin_name,
          "_<dir>\" directory.\nChange into that directory and try \"",
          cmd_name,
          " --run <dir>\" again.");
    }

    // Set this application as the app name for the moose_test_runner script that we're running
    setenv("MOOSE_TEST_RUNNER_APP_NAME", appBinaryName().c_str(), true);

    const std::string cmd = MooseUtils::runTestsExecutable() + test_args;
    Moose::out << "Working Directory: " << working_dir << "\nRunning Command: " << cmd << std::endl;
    mooseAssert(comm().size() == 1, "Should be run in serial");
    const auto return_value = system(cmd.c_str());
    if (!WIFEXITED(return_value))
      mooseError("Process exited unexpectedly");
    setExitCode(WEXITSTATUS(return_value));
    return true;
  }

  return false;
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
    {
      // Unless file_base was explicitly set by user, we cannot rely on it, as it will be changed
      // later
      const std::string cp_dir =
          _file_base_set_by_user ? params.get<std::string>("file_base")
                                 : (getOutputFileBase(true) + "_" + moose_object_action->name());
      checkpoint_dirs.push_back(cp_dir + "_cp");
    }
  }
  return checkpoint_dirs;
}

std::list<std::string>
MooseApp::getCheckpointFiles() const
{
  auto checkpoint_dirs = getCheckpointDirectories();
  return MooseUtils::getFilesInDirs(checkpoint_dirs, false);
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
  return _builder.getPrimaryFileName(stripLeadingPath);
}

OutputWarehouse &
MooseApp::getOutputWarehouse()
{
  return _output_warehouse;
}

const OutputWarehouse &
MooseApp::getOutputWarehouse() const
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
MooseApp::registerRestartableData(std::unique_ptr<RestartableDataValue> data,
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
  auto & data_map =
      metaname.empty() ? _restartable_data[tid] : _restartable_meta_data[metaname].first;

  RestartableDataValue * stored_data = data_map.findData(data->name());
  if (stored_data)
  {
    if (data->typeId() != stored_data->typeId())
      mooseError("Type mismatch found in RestartableData registration of '",
                 data->name(),
                 "'\n\n  Stored type: ",
                 stored_data->type(),
                 "\n  New type: ",
                 data->type());
  }
  else
    stored_data = &data_map.addData(std::move(data));

  if (!read_only)
    stored_data->setDeclared({});

  return *stored_data;
}

RestartableDataValue &
MooseApp::registerRestartableData(const std::string & libmesh_dbg_var(name),
                                  std::unique_ptr<RestartableDataValue> data,
                                  THREAD_ID tid,
                                  bool read_only,
                                  const RestartableDataMapName & metaname)
{
  mooseDeprecated("The use of MooseApp::registerRestartableData with a data name is "
                  "deprecated.\n\nUse the call without a name instead.");

  mooseAssert(name == data->name(), "Inconsistent name");
  return registerRestartableData(std::move(data), tid, read_only, metaname);
}

bool
MooseApp::hasRestartableMetaData(const std::string & name,
                                 const RestartableDataMapName & metaname) const
{
  auto it = _restartable_meta_data.find(metaname);
  if (it == _restartable_meta_data.end())
    return false;
  return it->second.first.hasData(name);
}

RestartableDataValue &
MooseApp::getRestartableMetaData(const std::string & name,
                                 const RestartableDataMapName & metaname,
                                 THREAD_ID tid)
{
  if (tid != 0)
    mooseError(
        "The meta data storage for '", metaname, "' is not threaded, so the tid must be zero.");

  // Get metadata reference from RestartableDataMap and return a (non-const) reference to its value
  auto & restartable_data_map = getRestartableDataMap(metaname);
  RestartableDataValue * const data = restartable_data_map.findData(name);
  if (!data)
    mooseError("Unable to find RestartableDataValue object with name " + name +
               " in RestartableDataMap");

  return *data;
}

void
MooseApp::possiblyLoadRestartableMetaData(const RestartableDataMapName & name,
                                          const std::filesystem::path & folder_base)
{
  const auto & map_name = getRestartableDataMapName(name);
  const auto meta_data_folder_base = metaDataFolderBase(folder_base, map_name);
  if (RestartableDataReader::isAvailable(meta_data_folder_base))
  {
    RestartableDataReader reader(*this, getRestartableDataMap(name));
    reader.setErrorOnLoadWithDifferentNumberOfProcessors(false);
    reader.setInput(meta_data_folder_base);
    reader.restore();
  }
}

void
MooseApp::loadRestartableMetaData(const std::filesystem::path & folder_base)
{
  for (const auto & name_map_pair : _restartable_meta_data)
    possiblyLoadRestartableMetaData(name_map_pair.first, folder_base);
}

std::vector<std::filesystem::path>
MooseApp::writeRestartableMetaData(const RestartableDataMapName & name,
                                   const std::filesystem::path & folder_base)
{
  if (processor_id() != 0)
    mooseError("MooseApp::writeRestartableMetaData(): Should only run on processor 0");

  const auto & map_name = getRestartableDataMapName(name);
  const auto meta_data_folder_base = metaDataFolderBase(folder_base, map_name);

  RestartableDataWriter writer(*this, getRestartableDataMap(name));
  return writer.write(meta_data_folder_base);
}

std::vector<std::filesystem::path>
MooseApp::writeRestartableMetaData(const std::filesystem::path & folder_base)
{
  std::vector<std::filesystem::path> paths;

  if (processor_id() == 0)
    for (const auto & name_map_pair : _restartable_meta_data)
    {
      const auto map_paths = writeRestartableMetaData(name_map_pair.first, folder_base);
      paths.insert(paths.end(), map_paths.begin(), map_paths.end());
    }

  return paths;
}

void
MooseApp::dynamicAppRegistration(const std::string & app_name,
                                 std::string library_path,
                                 const std::string & library_name,
                                 bool lib_load_deps)
{
#ifdef LIBMESH_HAVE_DLOPEN
  Parameters params;
  params.set<std::string>("app_name") = app_name;
  params.set<RegistrationType>("reg_type") = APPLICATION;
  params.set<std::string>("registration_method") = app_name + "__registerApps";
  params.set<std::string>("library_path") = library_path;

  const auto effective_library_name =
      library_name.empty() ? appNameToLibName(app_name) : library_name;
  params.set<std::string>("library_name") = effective_library_name;
  params.set<bool>("library_load_dependencies") = lib_load_deps;

  const auto paths = getLibrarySearchPaths(library_path);
  std::ostringstream oss;

  auto successfully_loaded = false;
  if (paths.empty())
    oss << '"' << app_name << "\" is not a registered application name.\n"
        << "No search paths were set. We made no attempts to locate the corresponding library "
           "file.\n";
  else
  {
    dynamicRegistration(params);

    // At this point the application should be registered so check it
    if (!AppFactory::instance().isRegistered(app_name))
    {
      oss << '"' << app_name << "\" is not a registered application name.\n"
          << "Unable to locate library archive for \"" << app_name
          << "\".\nWe attempted to locate the library archive \"" << effective_library_name
          << "\" in the following paths:\n\t";
      std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
    }
    else
      successfully_loaded = true;
  }

  if (!successfully_loaded)
  {
    oss << "\nMake sure you have compiled the library and either set the \"library_path\" "
           "variable in your input file or exported \"MOOSE_LIBRARY_PATH\".\n";

    mooseError(oss.str());
  }

#else
  libmesh_ignore(app_name, library_path, library_name, lib_load_deps);
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
  params.set<std::string>("library_name") =
      library_name.empty() ? appNameToLibName(app_name) : library_name;

  params.set<Factory *>("factory") = factory;
  params.set<Syntax *>("syntax") = syntax;
  params.set<ActionFactory *>("action_factory") = action_factory;
  params.set<bool>("library_load_dependencies") = false;

  dynamicRegistration(params);
#else
  libmesh_ignore(app_name, factory, action_factory, syntax, library_path, library_name);
  mooseError("Dynamic Loading is either not supported or was not detected by libMesh configure.");
#endif
}

void
MooseApp::dynamicRegistration(const Parameters & params)
{
  const auto paths = getLibrarySearchPaths(params.get<std::string>("library_path"));
  const auto library_name = params.get<std::string>("library_name");

  // Attempt to dynamically load the library
  for (const auto & path : paths)
    if (MooseUtils::checkFileReadable(path + '/' + library_name, false, false))
      loadLibraryAndDependencies(
          path + '/' + library_name, params, params.get<bool>("library_load_dependencies"));
}

void
MooseApp::loadLibraryAndDependencies(const std::string & library_filename,
                                     const Parameters & params,
                                     const bool load_dependencies)
{
  std::string line;
  std::string dl_lib_filename;

  // This RE looks for absolute path libtool filenames (i.e. begins with a slash and ends with a
  // .la)
  pcrecpp::RE re_deps("(/\\S*\\.la)");

  std::ifstream la_handle(library_filename.c_str());
  if (la_handle.is_open())
  {
    while (std::getline(la_handle, line))
    {
      // Look for the system dependent dynamic library filename to open
      if (line.find("dlname=") != std::string::npos)
        // Magic numbers are computed from length of this string "dlname=' and line minus that
        // string plus quotes"
        dl_lib_filename = line.substr(8, line.size() - 9);

      if (line.find("dependency_libs=") != std::string::npos)
      {
        if (load_dependencies)
        {
          pcrecpp::StringPiece input(line);
          pcrecpp::StringPiece depend_library;
          while (re_deps.FindAndConsume(&input, &depend_library))
            // Recurse here to load dependent libraries in depth-first order
            loadLibraryAndDependencies(depend_library.as_string(), params, load_dependencies);
        }

        // There's only one line in the .la file containing the dependency libs so break after
        // finding it
        break;
      }
    }
    la_handle.close();
  }

  // This should only occur if we have static linkage.
  if (dl_lib_filename.empty())
    return;

  const auto & [dir, file_name] = MooseUtils::splitFileName(library_filename);

  // Time to load the library, First see if we've already loaded this particular dynamic library
  //     1) make sure we haven't already loaded this library
  // AND 2) make sure we have a library name (we won't for static linkage)
  // Note: Here was are going to assume uniqueness based on the filename alone. This has significant
  // implications for applications that have "diamond" inheritance of libraries (usually
  // modules). We will only load one of those libraries, versions be damned.
  auto dyn_lib_it = _lib_handles.find(file_name);
  if (dyn_lib_it == _lib_handles.end())
  {
    // Assemble the actual filename using the base path of the *.la file and the dl_lib_filename
    const auto dl_lib_full_path = MooseUtils::pathjoin(dir, dl_lib_filename);

    MooseUtils::checkFileReadable(dl_lib_full_path, false, /*throw_on_unreadable=*/true);

#ifdef LIBMESH_HAVE_DLOPEN
    void * const lib_handle = dlopen(dl_lib_full_path.c_str(), RTLD_LAZY);
#else
    void * const lib_handle = nullptr;
#endif

    if (!lib_handle)
      mooseError("The library file \"",
                 dl_lib_full_path,
                 "\" exists and has proper permissions, but cannot by dynamically loaded.\nThis "
                 "generally means that the loader was unable to load one or more of the "
                 "dependencies listed in the supplied library (see otool or ldd).\n",
                 dlerror());

    DynamicLibraryInfo lib_info;
    lib_info.library_handle = lib_handle;
    lib_info.full_path = library_filename;

    auto insert_ret = _lib_handles.insert(std::make_pair(file_name, lib_info));
    mooseAssert(insert_ret.second == true, "Error inserting into lib_handles map");

    dyn_lib_it = insert_ret.first;
  }

  // Library has been loaded, check to see if we've called the requested registration method
  const auto registration_method = params.get<std::string>("registration_method");
  auto & entry_sym_from_curr_lib = dyn_lib_it->second.entry_symbols;

  if (entry_sym_from_curr_lib.find(registration_method) == entry_sym_from_curr_lib.end())
  {
    // get the pointer to the method in the library.  The dlsym()
    // function returns a null pointer if the symbol cannot be found,
    // we also explicitly set the pointer to NULL if dlsym is not
    // available.
#ifdef LIBMESH_HAVE_DLOPEN
    void * registration_handle =
        dlsym(dyn_lib_it->second.library_handle, registration_method.c_str());
#else
    void * registration_handle = nullptr;
#endif

    if (registration_handle)
    {
      switch (params.get<RegistrationType>("reg_type"))
      {
        case APPLICATION:
        {
          using register_app_t = void (*)();
          register_app_t * const reg_ptr = reinterpret_cast<register_app_t *>(&registration_handle);
          (*reg_ptr)();
          break;
        }
        case REGALL:
        {
          using register_app_t = void (*)(Factory *, ActionFactory *, Syntax *);
          register_app_t * const reg_ptr = reinterpret_cast<register_app_t *>(&registration_handle);
          (*reg_ptr)(params.get<Factory *>("factory"),
                     params.get<ActionFactory *>("action_factory"),
                     params.get<Syntax *>("syntax"));
          break;
        }
        default:
          mooseError("Unhandled RegistrationType");
      }

      entry_sym_from_curr_lib.insert(registration_method);
    }
    else
    {

#if defined(DEBUG) && defined(LIBMESH_HAVE_DLOPEN)
      // We found a dynamic library that doesn't have a dynamic
      // registration method in it. This shouldn't be an error, so
      // we'll just move on.
      if (!registration_handle)
        mooseWarning("Unable to find extern \"C\" method \"",
                     registration_method,
                     "\" in library: ",
                     dyn_lib_it->first,
                     ".\n",
                     "This doesn't necessarily indicate an error condition unless you believe that "
                     "the method should exist in that library.\n",
                     dlerror());
#endif
    }
  }
}

std::set<std::string>
MooseApp::getLoadedLibraryPaths() const
{
  // Return the paths but not the open file handles
  std::set<std::string> paths;
  for (const auto & it : _lib_handles)
    paths.insert(it.first);

  return paths;
}

std::set<std::string>
MooseApp::getLibrarySearchPaths(const std::string & library_path) const
{
  std::set<std::string> paths;

  if (!library_path.empty())
  {
    std::vector<std::string> tmp_paths;
    MooseUtils::tokenize(library_path, tmp_paths, 1, ":");

    paths.insert(tmp_paths.begin(), tmp_paths.end());
  }

  char * moose_lib_path_env = std::getenv("MOOSE_LIBRARY_PATH");
  if (moose_lib_path_env)
  {
    std::string moose_lib_path(moose_lib_path_env);
    std::vector<std::string> tmp_paths;
    MooseUtils::tokenize(moose_lib_path, tmp_paths, 1, ":");

    paths.insert(tmp_paths.begin(), tmp_paths.end());
  }

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
MooseApp::createMinimalApp()
{
  TIME_SECTION("createMinimalApp", 3, "Creating Minimal App");

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

bool
MooseApp::hasRelationshipManager(const std::string & name) const
{
  return std::find_if(_relationship_managers.begin(),
                      _relationship_managers.end(),
                      [&name](const std::shared_ptr<RelationshipManager> & rm)
                      { return rm->name() == name; }) != _relationship_managers.end();
}

namespace
{
void
donateForWhom(const RelationshipManager & donor, RelationshipManager & acceptor)
{
  auto & existing_for_whom = acceptor.forWhom();

  // Take all the for_whoms from the donor, and give them to the acceptor
  for (auto & fw : donor.forWhom())
  {
    if (std::find(existing_for_whom.begin(), existing_for_whom.end(), fw) ==
        existing_for_whom.end())
      acceptor.addForWhom(fw);
  }
}
}

bool
MooseApp::addRelationshipManager(std::shared_ptr<RelationshipManager> new_rm)
{
  // We prefer to always add geometric RMs. There is no hurt to add RMs for replicated mesh
  // since MeshBase::delete_remote_elements{} is a no-op (empty) for replicated mesh.
  // The motivation here is that MooseMesh::_use_distributed_mesh may not be properly set
  // at the time we are adding geometric relationship managers. We deleted the following
  // old logic to add all geometric RMs regardless of there is a distributed mesh or not.
  // Otherwise, all geometric RMs will be improperly ignored for a distributed mesh generator.

  // if (!_action_warehouse.mesh()->isDistributedMesh() && !_split_mesh &&
  //    (relationship_manager->isType(Moose::RelationshipManagerType::GEOMETRIC) &&
  //     !(relationship_manager->isType(Moose::RelationshipManagerType::ALGEBRAIC) ||
  //       relationship_manager->isType(Moose::RelationshipManagerType::COUPLING))))
  //  return false;

  bool add = true;

  std::set<std::shared_ptr<RelationshipManager>> rms_to_erase;

  for (const auto & existing_rm : _relationship_managers)
  {
    if (*existing_rm >= *new_rm)
    {
      add = false;
      donateForWhom(*new_rm, *existing_rm);
      break;
    }
    // The new rm did not provide less or the same amount/type of ghosting as the existing rm, but
    // what about the other way around?
    else if (*new_rm >= *existing_rm)
      rms_to_erase.emplace(existing_rm);
  }

  if (add)
  {
    _relationship_managers.emplace(new_rm);
    for (const auto & rm_to_erase : rms_to_erase)
    {
      donateForWhom(*rm_to_erase, *new_rm);
      removeRelationshipManager(rm_to_erase);
    }
  }

  // Inform the caller whether the object was added or not
  return add;
}

const std::string &
MooseApp::checkpointSuffix()
{
  static const std::string suffix = "-mesh.cpa.gz";
  return suffix;
}

std::filesystem::path
MooseApp::metaDataFolderBase(const std::filesystem::path & folder_base,
                             const std::string & map_suffix)
{
  return RestartableDataIO::restartableDataFolder(folder_base /
                                                  std::filesystem::path("meta_data" + map_suffix));
}

std::filesystem::path
MooseApp::restartFolderBase(const std::filesystem::path & folder_base) const
{
  auto folder = folder_base;
  folder += "-restart-" + std::to_string(processor_id());
  return RestartableDataIO::restartableDataFolder(folder);
}

const hit::Node *
MooseApp::getCurrentActionHitNode() const
{
  if (const auto action = _action_warehouse.getCurrentAction())
    return action->parameters().getHitNode();
  return nullptr;
}

bool
MooseApp::hasRMClone(const RelationshipManager & template_rm, const MeshBase & mesh) const
{
  auto it = _template_to_clones.find(&template_rm);
  // C++ does short circuiting so we're safe here
  return (it != _template_to_clones.end()) && (it->second.find(&mesh) != it->second.end());
}

RelationshipManager &
MooseApp::getRMClone(const RelationshipManager & template_rm, const MeshBase & mesh) const
{
  auto outer_it = _template_to_clones.find(&template_rm);
  if (outer_it == _template_to_clones.end())
    mooseError("The template rm does not exist in our _template_to_clones map");

  auto & mesh_to_clone_map = outer_it->second;
  auto inner_it = mesh_to_clone_map.find(&mesh);
  if (inner_it == mesh_to_clone_map.end())
    mooseError("We should have the mesh key in our mesh");

  return *inner_it->second;
}

void
MooseApp::removeRelationshipManager(std::shared_ptr<RelationshipManager> rm)
{
  auto * const mesh = _action_warehouse.mesh().get();
  if (!mesh)
    mooseError("The MooseMesh should exist");

  const MeshBase * const undisp_lm_mesh = mesh->getMeshPtr();
  RelationshipManager * undisp_clone = nullptr;
  if (undisp_lm_mesh && hasRMClone(*rm, *undisp_lm_mesh))
  {
    undisp_clone = &getRMClone(*rm, *undisp_lm_mesh);
    const_cast<MeshBase *>(undisp_lm_mesh)->remove_ghosting_functor(*undisp_clone);
  }

  auto & displaced_mesh = _action_warehouse.displacedMesh();
  MeshBase * const disp_lm_mesh = displaced_mesh ? &displaced_mesh->getMesh() : nullptr;
  RelationshipManager * disp_clone = nullptr;
  if (disp_lm_mesh && hasRMClone(*rm, *disp_lm_mesh))
  {
    disp_clone = &getRMClone(*rm, *disp_lm_mesh);
    disp_lm_mesh->remove_ghosting_functor(*disp_clone);
  }

  if (_executioner)
  {
    auto & problem = feProblem();
    if (undisp_clone)
    {
      problem.removeAlgebraicGhostingFunctor(*undisp_clone);
      problem.removeCouplingGhostingFunctor(*undisp_clone);
    }

    auto * dp = problem.getDisplacedProblem().get();
    if (dp && disp_clone)
      dp->removeAlgebraicGhostingFunctor(*disp_clone);
  }

  _factory.releaseSharedObjects(*rm);
  _relationship_managers.erase(rm);
}

RelationshipManager &
MooseApp::createRMFromTemplateAndInit(const RelationshipManager & template_rm,
                                      MooseMesh & moose_mesh,
                                      MeshBase & mesh,
                                      const DofMap * const dof_map)
{
  auto & mesh_to_clone = _template_to_clones[&template_rm];
  auto it = mesh_to_clone.find(&mesh);
  if (it != mesh_to_clone.end())
  {
    // We've already created a clone for this mesh
    auto & clone_rm = *it->second;
    if (!clone_rm.dofMap() && dof_map)
      // We didn't have a DofMap before, but now we do, so we should re-init
      clone_rm.init(moose_mesh, mesh, dof_map);
    else if (clone_rm.dofMap() && dof_map && (clone_rm.dofMap() != dof_map))
      mooseError("Attempting to create and initialize an existing clone with a different DofMap. "
                 "This should not happen.");

    return clone_rm;
  }

  // It's possible that this method is going to get called for multiple different MeshBase
  // objects. If that happens, then we *cannot* risk having a MeshBase object with a ghosting
  // functor that is init'd with another MeshBase object. So the safe thing to do is to make a
  // different RM for every MeshBase object that gets called here. Then the
  // RelationshipManagers stored here in MooseApp are serving as a template only
  auto pr = mesh_to_clone.emplace(
      std::make_pair(&const_cast<const MeshBase &>(mesh),
                     dynamic_pointer_cast<RelationshipManager>(template_rm.clone())));
  mooseAssert(pr.second, "An insertion should have happened");
  auto & clone_rm = *pr.first->second;
  clone_rm.init(moose_mesh, mesh, dof_map);
  return clone_rm;
}

void
MooseApp::attachRelationshipManagers(MeshBase & mesh, MooseMesh & moose_mesh)
{
  for (auto & rm : _relationship_managers)
  {
    if (rm->isType(Moose::RelationshipManagerType::GEOMETRIC))
    {
      if (rm->attachGeometricEarly())
        mesh.add_ghosting_functor(createRMFromTemplateAndInit(*rm, moose_mesh, mesh));
      else
      {
        // If we have a geometric ghosting functor that can't be attached early, then we have to
        // prevent the mesh from deleting remote elements
        moose_mesh.allowRemoteElementRemoval(false);

        if (const MeshBase * const moose_mesh_base = moose_mesh.getMeshPtr())
        {
          if (moose_mesh_base != &mesh)
            mooseError("The MooseMesh MeshBase and the MeshBase we're trying to attach "
                       "relationship managers to are different");
        }
        else
          // The MeshBase isn't attached to the MooseMesh yet, so have to tell it not to remove
          // remote elements independently
          mesh.allow_remote_element_removal(false);
      }
    }
  }
}

void
MooseApp::attachRelationshipManagers(Moose::RelationshipManagerType rm_type,
                                     bool attach_geometric_rm_final)
{
  for (auto & rm : _relationship_managers)
  {
    if (!rm->isType(rm_type))
      continue;

    // RM is already attached (this also handles the geometric early case)
    if (_attached_relationship_managers[rm_type].count(rm.get()))
      continue;

    if (rm_type == Moose::RelationshipManagerType::GEOMETRIC)
    {
      // The problem is not built yet - so the ActionWarehouse currently owns the mesh
      MooseMesh * const mesh = _action_warehouse.mesh().get();

      // "attach_geometric_rm_final = true" inidicate that it is the last chance to attach
      // geometric RMs. Therefore, we need to attach them.
      if (!rm->attachGeometricEarly() && !attach_geometric_rm_final)
        // Will attach them later (during algebraic). But also, we need to tell the mesh that we
        // shouldn't be deleting remote elements yet
        mesh->allowRemoteElementRemoval(false);
      else
      {
        MeshBase & undisp_mesh_base = mesh->getMesh();
        const DofMap * const undisp_sys_dof_map =
            _executioner ? &feProblem().getSolverSystem(0).dofMap() : nullptr;
        undisp_mesh_base.add_ghosting_functor(
            createRMFromTemplateAndInit(*rm, *mesh, undisp_mesh_base, undisp_sys_dof_map));

        // In the final stage, if there is a displaced mesh, we need to
        // clone ghosting functors for displacedMesh
        if (auto & disp_moose_mesh = _action_warehouse.displacedMesh();
            attach_geometric_rm_final && disp_moose_mesh)
        {
          MeshBase & disp_mesh_base = _action_warehouse.displacedMesh()->getMesh();
          const DofMap * disp_sys_dof_map = nullptr;
          if (_executioner && feProblem().getDisplacedProblem())
            disp_sys_dof_map = &feProblem().getDisplacedProblem()->solverSys(0).dofMap();
          disp_mesh_base.add_ghosting_functor(
              createRMFromTemplateAndInit(*rm, *disp_moose_mesh, disp_mesh_base, disp_sys_dof_map));
        }
        else if (_action_warehouse.displacedMesh())
          mooseError("The displaced mesh should not yet exist at the time that we are attaching "
                     "early geometric relationship managers.");

        // Mark this RM as attached
        mooseAssert(!_attached_relationship_managers[rm_type].count(rm.get()), "Already attached");
        _attached_relationship_managers[rm_type].insert(rm.get());
      }
    }
    else // rm_type is algebraic or coupling
    {
      if (!_executioner && !_executor)
        mooseError("We must have an executioner by now or else we do not have to data to add "
                   "algebraic or coupling functors to in MooseApp::attachRelationshipManagers");

      // Now we've built the problem, so we can use it
      auto & problem = feProblem();
      auto & undisp_moose_mesh = problem.mesh();
      auto & undisp_sys = feProblem().getSolverSystem(0);
      auto & undisp_sys_dof_map = undisp_sys.dofMap();
      auto & undisp_mesh = undisp_moose_mesh.getMesh();

      if (rm->useDisplacedMesh() && problem.getDisplacedProblem())
      {
        if (rm_type == Moose::RelationshipManagerType::COUPLING)
          // We actually need to add this to the FEProblemBase NonlinearSystemBase's DofMap
          // because the DisplacedProblem "nonlinear" DisplacedSystem doesn't have any matrices
          // for which to do coupling. It's actually horrifying to me that we are adding a
          // coupling functor, that is going to determine its couplings based on a displaced
          // MeshBase object, to a System associated with the undisplaced MeshBase object (there
          // is only ever one EquationSystems object per MeshBase object and visa versa). So here
          // I'm left with the choice of whether to pass in a MeshBase object that is *not* the
          // MeshBase object that will actually determine the couplings or to pass in the MeshBase
          // object that is inconsistent with the System DofMap that we are adding the coupling
          // functor for! Let's err on the side of *libMesh* consistency and pass properly paired
          // MeshBase-DofMap
          problem.addCouplingGhostingFunctor(
              createRMFromTemplateAndInit(*rm, undisp_moose_mesh, undisp_mesh, &undisp_sys_dof_map),
              /*to_mesh = */ false);

        else if (rm_type == Moose::RelationshipManagerType::ALGEBRAIC)
        {
          auto & displaced_problem = *problem.getDisplacedProblem();
          auto & disp_moose_mesh = displaced_problem.mesh();
          auto & disp_mesh = disp_moose_mesh.getMesh();
          const DofMap * const disp_nl_dof_map = &displaced_problem.solverSys(0).dofMap();
          displaced_problem.addAlgebraicGhostingFunctor(
              createRMFromTemplateAndInit(*rm, disp_moose_mesh, disp_mesh, disp_nl_dof_map),
              /*to_mesh = */ false);
        }
      }
      else // undisplaced
      {
        if (rm_type == Moose::RelationshipManagerType::COUPLING)
          problem.addCouplingGhostingFunctor(
              createRMFromTemplateAndInit(*rm, undisp_moose_mesh, undisp_mesh, &undisp_sys_dof_map),
              /*to_mesh = */ false);

        else if (rm_type == Moose::RelationshipManagerType::ALGEBRAIC)
          problem.addAlgebraicGhostingFunctor(
              createRMFromTemplateAndInit(*rm, undisp_moose_mesh, undisp_mesh, &undisp_sys_dof_map),
              /*to_mesh = */ false);
      }

      // Mark this RM as attached
      mooseAssert(!_attached_relationship_managers[rm_type].count(rm.get()), "Already attached");
      _attached_relationship_managers[rm_type].insert(rm.get());
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
    // Let us use an ordered map to avoid stochastic console behaviors.
    // I believe we won't have many RMs, and there is no performance issue.
    // Deterministic behaviors are good for setting up regression tests
    std::map<std::string, unsigned int> counts;

    for (auto & gf : as_range(mesh->getMesh().ghosting_functors_begin(),
                              mesh->getMesh().ghosting_functors_end()))
    {
      const auto * gf_ptr = dynamic_cast<const RelationshipManager *>(gf);
      if (!gf_ptr)
        // Count how many occurences of the same Ghosting Functor types we are encountering
        counts[demangle(typeid(*gf).name())]++;
    }

    for (const auto & pair : counts)
      info_strings.emplace_back(std::make_pair(
          "Default", pair.first + (pair.second > 1 ? " x " + std::to_string(pair.second) : "")));
  }

  // List the libMesh GhostingFunctors - Not that in libMesh all of the algebraic and coupling
  // Ghosting Functors are also attached to the mesh. This should catch them all.
  const auto & d_mesh = _action_warehouse.getDisplacedMesh();
  if (d_mesh)
  {
    // Let us use an ordered map to avoid stochastic console behaviors.
    // I believe we won't have many RMs, and there is no performance issue.
    // Deterministic behaviors are good for setting up regression tests
    std::map<std::string, unsigned int> counts;

    for (auto & gf : as_range(d_mesh->getMesh().ghosting_functors_begin(),
                              d_mesh->getMesh().ghosting_functors_end()))
    {
      const auto * gf_ptr = dynamic_cast<const RelationshipManager *>(gf);
      if (!gf_ptr)
        // Count how many occurences of the same Ghosting Functor types we are encountering
        counts[demangle(typeid(*gf).name())]++;
    }

    for (const auto & pair : counts)
      info_strings.emplace_back(
          std::make_pair("Default",
                         pair.first + (pair.second > 1 ? " x " + std::to_string(pair.second) : "") +
                             " for DisplacedMesh"));
  }

  return info_strings;
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

    for (const auto & data : meta_data)
      if (!data.declared())
        not_declared.push_back(data.name());

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
const RestartableDataMapName MooseApp::MESH_META_DATA_SUFFIX = "mesh";

RestartableDataMap &
MooseApp::getRestartableDataMap(const RestartableDataMapName & name)
{
  auto iter = _restartable_meta_data.find(name);
  if (iter == _restartable_meta_data.end())
    mooseError("Unable to find RestartableDataMap object for the supplied name '",
               name,
               "', did you call registerRestartableDataMapName in the application constructor?");
  return iter->second.first;
}

bool
MooseApp::hasRestartableDataMap(const RestartableDataMapName & name) const
{
  return _restartable_meta_data.count(name);
}

void
MooseApp::registerRestartableDataMapName(const RestartableDataMapName & name, std::string suffix)
{
  if (!suffix.empty())
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
  suffix.insert(0, "_");
  _restartable_meta_data.emplace(
      std::make_pair(name, std::make_pair(RestartableDataMap(), suffix)));
}

const std::string &
MooseApp::getRestartableDataMapName(const RestartableDataMapName & name) const
{
  const auto it = _restartable_meta_data.find(name);
  if (it == _restartable_meta_data.end())
    mooseError("MooseApp::getRestartableDataMapName: The name '", name, "' is not registered");
  return it->second.second;
}

PerfGraph &
MooseApp::createRecoverablePerfGraph()
{
  registerRestartableNameWithFilter("perf_graph", Moose::RESTARTABLE_FILTER::RECOVERABLE);

  auto perf_graph =
      std::make_unique<RestartableData<PerfGraph>>("perf_graph",
                                                   this,
                                                   type() + " (" + name() + ')',
                                                   *this,
                                                   getParam<bool>("perf_graph_live_all"),
                                                   !getParam<bool>("disable_perf_graph_live"));

  return dynamic_cast<RestartableData<PerfGraph> &>(
             registerRestartableData(std::move(perf_graph), 0, false))
      .set();
}

SolutionInvalidity &
MooseApp::createRecoverableSolutionInvalidity()
{
  registerRestartableNameWithFilter("solution_invalidity", Moose::RESTARTABLE_FILTER::RECOVERABLE);

  auto solution_invalidity =
      std::make_unique<RestartableData<SolutionInvalidity>>("solution_invalidity", nullptr, *this);

  return dynamic_cast<RestartableData<SolutionInvalidity> &>(
             registerRestartableData(std::move(solution_invalidity), 0, false))
      .set();
}

bool
MooseApp::constructingMeshGenerators() const
{
  return _action_warehouse.getCurrentTaskName() == "create_added_mesh_generators" ||
         _mesh_generator_system.appendingMeshGenerators();
}

#ifdef LIBTORCH_ENABLED
torch::DeviceType
MooseApp::determineLibtorchDeviceType(const MooseEnum & device_enum) const
{
  if (device_enum == "cuda")
  {
#ifdef __linux__
    if (!torch::cuda::is_available())
      mooseError("--libtorch-device=cuda: CUDA is not available");
    return torch::kCUDA;
#else
    mooseError("--libtorch-device=cuda: CUDA is not supported on your platform");
#endif
  }
  else if (device_enum == "mps")
  {
#ifdef __APPLE__
    if (!torch::mps::is_available())
      mooseError("--libtorch-device=mps: MPS is not available");
    return torch::kMPS;
#else
    mooseError("--libtorch-device=mps: MPS is not supported on your platform");
#endif
  }

  mooseAssert(device_enum == "cpu", "Should be cpu");
  return torch::kCPU;
}
#endif
