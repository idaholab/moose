//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef LIBTORCH_ENABLED
// Libtorch includes
#include <torch/types.h>
#include <torch/mps.h>
#include <torch/cuda.h>
#include <c10/core/DeviceType.h>
#endif

// MOOSE includes
#include "Moose.h"
#include "Parser.h"
#include "Builder.h"
#include "ActionWarehouse.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "OutputWarehouse.h"
#include "RestartableData.h"
#include "RestartableDataMap.h"
#include "ConsoleStreamInterface.h"
#include "PerfGraph.h"
#include "PerfGraphInterface.h"
#include "TheWarehouse.h"
#include "RankMap.h"
#include "MeshGeneratorSystem.h"
#include "ChainControlDataSystem.h"
#include "RestartableDataReader.h"
#include "Backup.h"
#include "MooseBase.h"
#include "libmesh/parallel_object.h"
#include "libmesh/mesh_base.h"
#include "libmesh/point.h"

// C++ includes
#include <list>
#include <map>
#include <set>
#include <unordered_set>
#include <typeindex>
#include <filesystem>

// Forward declarations
class Executioner;
class Executor;
class NullExecutor;
class FEProblemBase;
class InputParameterWarehouse;
class SystemInfo;
class CommandLine;
class RelationshipManager;
class SolutionInvalidity;

namespace libMesh
{
class ExodusII_IO;
}
namespace hit
{
class Node;
}

/**
 * Base class for MOOSE-based applications
 *
 * This generic class for application provides:
 * - parsing command line arguments,
 * - parsing an input file,
 * - executing the simulation
 *
 * Each application should register its own objects and register its own special syntax
 */
class MooseApp : public ConsoleStreamInterface,
                 public PerfGraphInterface,
                 public libMesh::ParallelObject,
                 public MooseBase
{
public:
#ifdef LIBTORCH_ENABLED
  /// Get the device torch is supposed to be running on.
  torch::DeviceType getLibtorchDevice() const { return _libtorch_device; }
#endif

  /**
   * Stores configuration options relating to the fixed-point solving
   * capability.  This is used for communicating input-file-based config from
   * the MultiApp object/syntax to the execution (e.g. executor) system.
   */
  struct FixedPointConfig
  {
    FixedPointConfig() : sub_relaxation_factor(1.0) {}
    /// relaxation factor to be used for a MultiApp's subapps.
    Real sub_relaxation_factor;
    /// The names of variables to transform for fixed point solve algorithms (e.g. secant, etc.).
    std::vector<std::string> sub_transformed_vars;
    /// The names of postprocessors to transform for fixed point solve algorithms (e.g. secant, etc.).
    std::vector<PostprocessorName> sub_transformed_pps;
  };

  static const RestartableDataMapName MESH_META_DATA;
  static const std::string MESH_META_DATA_SUFFIX;

  static InputParameters validParams();

  virtual ~MooseApp();

  TheWarehouse & theWarehouse() { return *_the_warehouse; }

  /**
   * Get printable name of the application.
   */
  virtual std::string getPrintableName() const { return "Application"; }

  virtual std::string appBinaryName() const
  {
    auto name = Moose::getExecutableName();
    name = name.substr(0, name.find_last_of("-"));
    if (name.find_first_of("/") != std::string::npos)
      name = name.substr(name.find_first_of("/") + 1, std::string::npos);
    return name;
  }

  /**
   * Get the shell exit code for the application
   * @return The shell exit code
   */
  int exitCode() const { return _exit_code; }

  /**
   * Sets the exit code that the application will exit with.
   */
  void setExitCode(const int exit_code) { _exit_code = exit_code; }

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  InputParameters & parameters() { return _pars; }

  /**
   * The RankMap is a useful object for determining how the processes
   * are laid out on the physical nodes of the cluster
   */
  const RankMap & rankMap() { return _rank_map; }

  /**
   * Get the PerfGraph for this app
   */
  PerfGraph & perfGraph() { return _perf_graph; }

  /**
   * Get the SolutionInvalidity for this app
   */
  ///@{
  SolutionInvalidity & solutionInvalidity() { return _solution_invalidity; }
  const SolutionInvalidity & solutionInvalidity() const { return _solution_invalidity; }
  ///@}

  ///@{
  /**
   * Retrieve a parameter for the object
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name);

  template <typename T>
  const T & getParam(const std::string & name) const;
  ///@}

  /**
   * Retrieve a renamed parameter for the object. This helper makes sure we
   * check both names before erroring, and that only one parameter is passed to avoid
   * silent errors
   * @param old_name the old name for the parameter
   * @param new_name the new name for the parameter
   */
  template <typename T>
  const T & getRenamedParam(const std::string & old_name, const std::string & new_name) const;

  inline bool isParamValid(const std::string & name) const { return _pars.isParamValid(name); }

  inline bool isParamSetByUser(const std::string & nm) const { return _pars.isParamSetByUser(nm); }

  /**
   * Run the application
   */
  virtual void run();

  /**
   * Returns the framework version.
   */
  std::string getFrameworkVersion() const;

  /**
   * Returns the current version of the framework or application (default: framework version).
   */
  virtual std::string getVersion() const;

  /**
   * Non-virtual method for printing out the version string in a consistent format.
   */
  std::string getPrintableVersion() const;

  /**
   * Setup options based on InputParameters.
   */
  virtual void setupOptions();

  /**
   * Return a writable reference to the ActionWarehouse associated with this app
   */
  ActionWarehouse & actionWarehouse() { return _action_warehouse; }
  /**
   * Return a const reference to the ActionWarehouse associated with this app
   */
  const ActionWarehouse & actionWarehouse() const { return _action_warehouse; }

  /**
   * Returns a writable reference to the builder
   */
  Moose::Builder & builder() { return _builder; }

  /**
   * Returns a writable reference to the syntax object.
   */
  Syntax & syntax() { return _syntax; }

  /**
   * @return the input file names set in the Parser
   */
  const std::vector<std::string> & getInputFileNames() const;

  /**
   * @return The last input filename set (if any)
   */
  const std::string & getLastInputFileName() const;

  /**
   * Override the selection of the output file base name.
   * Note: This method is supposed to be called by MultiApp only.
   */
  void setOutputFileBase(const std::string & output_file_base);

  /**
   * Get the output file base name.
   * @param for_non_moose_build_output True for getting the file base for outputs generated with
   *                                   Outputs/[outputname] input syntax.
   * @return The file base name used by output objects
   * Note: for_non_moose_build_output does not affect the returned value when this is a subapp.
   *       for_non_moose_build_output also does not affect the returned value when Outputs/file_base
   *       parameter is available. When for_non_moose_build_output does affect the returned value,
   *       i.e. master without Outputs/file_base, the suffix _out is removed.
   */
  std::string getOutputFileBase(bool for_non_moose_build_output = false) const;

  /**
   * Tell the app to output in a specific position.
   */
  void setOutputPosition(const Point & p);

  /**
   * Get all checkpoint directories
   * @return A Set of checkpoint directories
   */
  std::list<std::string> getCheckpointDirectories() const;

  /**
   * Extract all possible checkpoint file names
   * @return A Set of checkpoint filenames
   */
  std::list<std::string> getCheckpointFiles() const;

  /**
   * Whether or not an output position has been set.
   * @return True if it has
   */
  bool hasOutputPosition() const { return _output_position_set; }

  /**
   * Get the output position.
   * @return The position offset for the output.
   */
  Point getOutputPosition() const { return _output_position; }

  /**
   * Set the starting time for the simulation.  This will override any choice
   * made in the input file.
   *
   * @param time The start time for the simulation.
   */
  void setStartTime(Real time);

  /**
   * @return Whether or not a start time has been programmatically set using setStartTime()
   */
  bool hasStartTime() const { return _start_time_set; }

  /**
   * @return The start time
   */
  Real getStartTime() const { return _start_time; }

  /**
   * Each App has it's own local time.  The "global" time of the whole problem might be
   * different.  This offset is how far off the local App time is from the global time.
   */
  void setGlobalTimeOffset(Real offset) { _global_time_offset = offset; }

  /**
   * Each App has it's own local time.  The "global" time of the whole problem might be
   * different.  This offset is how far off the local App time is from the global time.
   */
  Real getGlobalTimeOffset() const { return _global_time_offset; }

  /**
   * Return the primary (first) filename that was parsed
   * Note: When stripLeadingPath is false, this function returns the same name as
   *       getInputFileName() method when the input file is not a link.
   */
  std::string getFileName(bool stripLeadingPath = true) const;

  /**
   * Set a flag so that the parser will throw an error if overridden parameters are detected
   */
  void setErrorOverridden();

  /**
   * Removes warnings and error checks for unrecognized variables in the input file
   */
  void disableCheckUnusedFlag();

  /**
   * Retrieve the Executioner for this App
   */
  Executioner * getExecutioner() const;
  Executor * getExecutor() const { return _executor.get(); }
  NullExecutor * getNullExecutor() const { return _null_executor.get(); }
  bool useExecutor() const { return _use_executor; }
  FEProblemBase & feProblem() const;

  /**
   * Set the Executioner for this App
   */
  void setExecutioner(std::shared_ptr<Executioner> && executioner) { _executioner = executioner; }
  void setExecutor(std::shared_ptr<Executor> && executor) { _executor = executor; }
  void
  addExecutor(const std::string & type, const std::string & name, const InputParameters & params);

  /**
   * Adds the parameters for an Executor to the list of parameters.  This is done
   * so that the Executors can be created in _exactly_ the correct order.
   */
  void addExecutorParams(const std::string & type,
                         const std::string & name,
                         const InputParameters & params);

  /**
   * @return The Parser
   **/
  Parser & parser();

private:
  /**
   * Internal function used to recursively create the executor objects.
   *
   * Called by createExecutors
   *
   * @param current_executor_name The name of the executor currently needing to be built
   * @param possible_roots The names of executors that are currently candidates for being the root
   */
  void recursivelyCreateExecutors(const std::string & current_executor_name,
                                  std::list<std::string> & possible_roots,
                                  std::list<std::string> & current_branch);

public:
  /**
   * After adding all of the Executor Params - this function will actually cause all of them to be
   * built
   */
  void createExecutors();

  /**
   * Get an Executor
   *
   * @param name The name of the Executor
   * @param fail_if_not_found Whether or not to fail if the executor doesn't exist.  If this is
   * false then this function will return a NullExecutor
   */
  Executor & getExecutor(const std::string & name, bool fail_if_not_found = true);

  /**
   * This info is stored here because we need a "globalish" place to put it in
   * order to allow communication between a multiapp and solver-specific
   * internals (i.e. relating to fixed-point inner loops like picard, etc.)
   * for handling subapp-specific modifications necessary for those solve
   * processes.
   */
  FixedPointConfig & fixedPointConfig() { return _fixed_point_config; }

  /**
   * Returns a writable Boolean indicating whether this app will use a Nonlinear or Eigen System.
   */
  bool & useNonlinear() { return _use_nonlinear; }

  /**
   * Returns a writable Boolean indicating whether this app will use an eigenvalue executioner.
   */
  bool & useEigenvalue() { return _use_eigen_value; }

  /**
   * Retrieve a writable reference to the Factory associated with this App.
   */
  Factory & getFactory() { return _factory; }

  /**
   * Retrieve a writable reference to the ActionFactory associated with this App.
   */
  ActionFactory & getActionFactory() { return _action_factory; }

  /**
   * Returns the MPI processor ID of the current processor.
   */
  processor_id_type processor_id() const { return _comm->rank(); }

  /**
   * Get the command line
   * @return The reference to the command line object
   * Setup options based on InputParameters.
   */
  std::shared_ptr<CommandLine> commandLine() const { return _command_line; }

  /**
   * Set the flag to indicate whether or not we need to use a separate Exodus reader to read the
   * mesh BEFORE we create the mesh.
   */
  void setExodusFileRestart(bool flag) { _initial_from_file = flag; }

  /**
   * Whether or not we need to use a separate Exodus reader to read the mesh BEFORE we create the
   * mesh.
   */
  bool getExodusFileRestart() const { return _initial_from_file; }

  /**
   * Set the Exodus reader to restart variables from an Exodus mesh file
   */
  void setExReaderForRestart(std::shared_ptr<libMesh::ExodusII_IO> && exreader)
  {
    _ex_reader = exreader;
  }

  /**
   * Get the Exodus reader to restart variables from an Exodus mesh file
   */
  libMesh::ExodusII_IO * getExReaderForRestart() const { return _ex_reader.get(); }

  /**
   * Actually build everything in the input file.
   */
  virtual void runInputFile();

  /**
   * Execute the Executioner that was built.
   */
  virtual void executeExecutioner();

  /**
   * Returns true if the user specified --distributed-mesh (or
   * --parallel-mesh, for backwards compatibility) on the command line
   * and false otherwise.
   */
  bool getDistributedMeshOnCommandLine() const { return _distributed_mesh_on_command_line; }

  /**
   * Whether or not this is a "recover" calculation. More specifically whether this simulation has
   * been recovered with something like the \p --recover command line argument. Note that this will
   * never return true when \p isRestarting is true
   */
  bool isRecovering() const;

  /**
   * Whether or not this is a "restart" calculation. More specifically whether this has been
   * restarted using the \p Problem/restart_file_base parameter. Note that this will only return
   * true when doing \emph checkpoint restart. This will be false if doing \emph exodus restart.
   * Finally this will never return true when \p isRecovering is true
   */
  bool isRestarting() const;

  /**
   * Whether or not this is a split mesh operation.
   */
  bool isSplitMesh() const;

  ///@{
  /**
   * Return true if the recovery file base is set
   */
  bool hasRestartRecoverFileBase() const;
  bool hasRecoverFileBase() const;
  ///@}

  ///@{
  /**
   * The file_base for the recovery file.
   */
  std::string getRestartRecoverFileBase() const { return _restart_recover_base; }
  std::string getRecoverFileBase() const
  {
    mooseDeprecated("MooseApp::getRecoverFileBase is deprecated, use "
                    "MooseApp::getRestartRecoverFileBase() instead.");
    return _restart_recover_base;
  }
  ///@}

  /**
   * mutator for recover_base (set by RecoverBaseAction)
   */
  void setRestartRecoverFileBase(const std::string & file_base)
  {
    if (file_base.empty())
      _restart_recover_base = MooseUtils::getLatestCheckpointFilePrefix(getCheckpointFiles());
    else
      _restart_recover_base = file_base;
  }

  /**
   *  Whether or not this simulation should only run half its transient (useful for testing
   * recovery)
   */
  bool testCheckpointHalfTransient() const { return _test_checkpoint_half_transient; }

  /**
   * Store a map of outputter names and file numbers
   * The MultiApp system requires this to get the file numbering to propagate down through the
   * Multiapps.
   * @param numbers Map of outputter names and file numbers
   *
   * @see MultiApp TransientMultiApp OutputWarehouse
   */
  void setOutputFileNumbers(const std::map<std::string, unsigned int> & numbers)
  {
    _output_file_numbers = numbers;
  }

  /**
   * Store a map of outputter names and file numbers
   * The MultiApp system requires this to get the file numbering to propogate down through the
   * multiapps.
   *
   * @see MultiApp TransientMultiApp
   */
  const std::map<std::string, unsigned int> & getOutputFileNumbers() const
  {
    return _output_file_numbers;
  }

  /**
   * Get the OutputWarehouse objects
   */
  OutputWarehouse & getOutputWarehouse();
  const OutputWarehouse & getOutputWarehouse() const;

  /**
   * Get SystemInfo object
   * @return A pointer to the SystemInformation object
   */
  const SystemInfo * getSystemInfo() const { return _sys_info.get(); }

  ///@{
  /**
   * Thes methods are called to register applications or objects on demand. This method
   * attempts to load a dynamic library and register it when it is needed. Throws an error if
   * no suitable library is found that contains the app_name in question.
   */
  void dynamicAllRegistration(const std::string & app_name,
                              Factory * factory,
                              ActionFactory * action_factory,
                              Syntax * syntax,
                              std::string library_path,
                              const std::string & library_name);
  void dynamicAppRegistration(const std::string & app_name,
                              std::string library_path,
                              const std::string & library_name,
                              bool lib_load_deps);
  ///@}

  /**
   * Converts an application name to a library name:
   * Examples:
   *   AnimalApp -> libanimal-oprof.la (assuming METHOD=oprof)
   *   ThreeWordAnimalApp -> libthree_word_animal-dbg.la (assuming METHOD=dbg)
   */
  std::string appNameToLibName(const std::string & app_name) const;

  /**
   * Converts a library name to an application name:
   */
  std::string libNameToAppName(const std::string & library_name) const;

  /**
   * Return the paths of loaded libraries
   */
  std::set<std::string> getLoadedLibraryPaths() const;

  /**
   * Return the paths searched by MOOSE when loading libraries
   */
  std::set<std::string> getLibrarySearchPaths(const std::string & library_path_from_param) const;

  /**
   * Get the InputParameterWarehouse for MooseObjects
   */
  InputParameterWarehouse & getInputParameterWarehouse();

  /*
   * Register a piece of restartable data.  This is data that will get
   * written / read to / from a restart file.
   *
   * @param data The actual data object.
   * @param tid The thread id of the object.  Use 0 if the object is not threaded.
   * @param read_only Restrict the data for read-only
   * @param metaname (optional) register the data to the meta data storage (tid must be 0)
   */
  RestartableDataValue & registerRestartableData(std::unique_ptr<RestartableDataValue> data,
                                                 THREAD_ID tid,
                                                 bool read_only,
                                                 const RestartableDataMapName & metaname = "");

  /*
   * Deprecated method to register a piece of restartable data.
   *
   * Use the call without a data name instead.
   */
  RestartableDataValue & registerRestartableData(const std::string & name,
                                                 std::unique_ptr<RestartableDataValue> data,
                                                 THREAD_ID tid,
                                                 bool read_only,
                                                 const RestartableDataMapName & metaname = "");

  /*
   * Check if a restartable meta data exists or not.
   *
   * @param name The full (unique) name.
   * @param metaname The name to the meta data storage
   */
  bool hasRestartableMetaData(const std::string & name,
                              const RestartableDataMapName & metaname) const;

  /*
   * Retrieve restartable meta data from restartable data map
   *
   * @param name The full (unique) name.
   * @param metaname The name to the meta data storage
   * @return A reference to the restartable meta data value
   */
  RestartableDataValue & getRestartableMetaData(const std::string & name,
                                                const RestartableDataMapName & metaname,
                                                THREAD_ID tid);

  /**
   * Loads the restartable meta data for \p name if it is available with the folder base \p
   * folder_base
   */
  void possiblyLoadRestartableMetaData(const RestartableDataMapName & name,
                                       const std::filesystem::path & folder_base);
  /**
   * Loads all available restartable meta data if it is available with the folder base \p
   * folder_base
   */
  void loadRestartableMetaData(const std::filesystem::path & folder_base);

  /**
   * Writes the restartable meta data for \p name with a folder base of \p folder_base
   *
   * @return The files that were written
   */
  std::vector<std::filesystem::path>
  writeRestartableMetaData(const RestartableDataMapName & name,
                           const std::filesystem::path & folder_base);
  /**
   * Writes all available restartable meta data with a file base of \p file_base
   *
   * @return The files that were written
   */
  std::vector<std::filesystem::path>
  writeRestartableMetaData(const std::filesystem::path & folder_base);

  /**
   * Return reference to the restartable data object
   * @return A reference to the restartable data object
   */
  ///@{
  const std::vector<RestartableDataMap> & getRestartableData() const { return _restartable_data; }
  std::vector<RestartableDataMap> & getRestartableData() { return _restartable_data; }
  ///@}

  /**
   * Return a reference to restartable data for the specific type flag.
   */
  RestartableDataMap & getRestartableDataMap(const RestartableDataMapName & name);

  /**
   * @return Whether or not the restartable data has the given name registered.
   */
  bool hasRestartableDataMap(const RestartableDataMapName & name) const;

  /**
   * Reserve a location for storing custom RestartableDataMap objects.
   *
   * This should be called in the constructor of an application.
   *
   * @param name A key to use for accessing the data object
   * @param suffix The suffix to use when appending to checkpoint output, if not supplied the
   *               given name is used to generate the suffix (MyMetaData -> _mymetadata)
   */
  void registerRestartableDataMapName(const RestartableDataMapName & name, std::string suffix = "");

  /**
   * @return The output name for the restartable data with name \p name
   */
  const std::string & getRestartableDataMapName(const RestartableDataMapName & name) const;

  /**
   * Return a reference to the recoverable data object
   * @return A const reference to the recoverable data
   */
  const DataNames & getRecoverableData() const { return _recoverable_data_names; }

  /**
   * Backs up the application to the folder \p folder_base
   *
   * @return The files that are written in the backup
   */
  std::vector<std::filesystem::path> backup(const std::filesystem::path & folder_base);
  /**
   * Backs up the application memory in a Backup.
   *
   * @return The backup
   */
  std::unique_ptr<Backup> backup();

  /**
   * Insertion point for other apps that is called before backup()
   */
  virtual void preBackup() {}

  /**
   * Restore an application from file

   * @param folder_base The backup folder base
   * @param for_restart Whether this restoration is explicitly for the first restoration of restart
   * data
   *
   * You must call finalizeRestore() after this in order to finalize the restoration.
   * The restore process is kept open in order to restore additional data after
   * the initial restore (that is, the restoration of data that has already been declared).
   */
  void restore(const std::filesystem::path & folder_base, const bool for_restart);

  /**
   * Restore an application from the backup \p backup
   *
   * @param backup The backup
   * @param for_restart Whether this restoration is explicitly for the first restoration of restart
   * data
   *
   * You must call finalizeRestore() after this in order to finalize the restoration.
   * The restore process is kept open in order to restore additional data after
   * the initial restore (that is, the restoration of data that has already been declared).
   */
  void restore(std::unique_ptr<Backup> backup, const bool for_restart);

  /**
   * Insertion point for other apps that is called after restore()
   *
   * @param for_restart Whether this restoration is explicitly for the
   * first restoration of restart data
   */
  virtual void postRestore(const bool /* for_restart */) {}

  /**
   * Restores from a "initial" backup, that is, one set in _initial_backup.
   *
   * @param for_restart Whether this restoration is explicitly for the first restoration of restart
   * data
   *
   * This is only used for restoration of multiapp subapps, which have been given
   * a Backup from their parent on initialization. Said Backup is passed to this app
   * via the "_initial_backup" private input parameter.
   *
   * See restore() for more information
   */
  void restoreFromInitialBackup(const bool for_restart);

  /**
   * Finalizes (closes) the restoration process done in restore().
   *
   * @return The underlying Backup that was used to do the restoration (if any, will be null when
   * backed up from file); can be ignored to destruct it
   *
   * This releases access to the stream in which the restore was loaded from
   * and makes it no longer possible to restore additional data.
   */
  std::unique_ptr<Backup> finalizeRestore();

  /**
   * Returns a string to be printed at the beginning of a simulation
   */
  virtual std::string header() const;

  /**
   * The MultiApp Level
   * @return The current number of levels from the master app
   */
  unsigned int multiAppLevel() const { return _multiapp_level; }

  /**
   * The MultiApp number
   * @return The numbering in all the sub-apps on the same level
   */
  unsigned int multiAppNumber() const { return _multiapp_number; }

  /**
   * Whether or not this app is the ultimate master app. (ie level == 0)
   */
  bool isUltimateMaster() const { return !_multiapp_level; }

  /**
   * Returns a pointer to the master mesh
   */
  const MooseMesh * masterMesh() const { return _master_mesh; }

  /**
   * Returns a pointer to the master displaced mesh
   */
  const MooseMesh * masterDisplacedMesh() const { return _master_displaced_mesh; }

  /**
   * Gets the system that manages the MeshGenerators
   */
  MeshGeneratorSystem & getMeshGeneratorSystem() { return _mesh_generator_system; }

  /**
   * Gets the system that manages the ChainControls
   */
  ChainControlDataSystem & getChainControlDataSystem() { return _chain_control_system; }

  /**
   * Add a mesh generator that will act on the meshes in the system
   *
   * @param type The type of MeshGenerator
   * @param name The name of the MeshGenerator
   * @param params The params used to construct the MeshGenerator
   *
   * See MeshGeneratorSystem::addMeshGenerator()
   */
  void addMeshGenerator(const std::string & type,
                        const std::string & name,
                        const InputParameters & params)
  {
    _mesh_generator_system.addMeshGenerator(type, name, params);
  }

  /**
   * @returns Whether or not a mesh generator exists with the name \p name.
   */
  bool hasMeshGenerator(const MeshGeneratorName & name) const
  {
    return _mesh_generator_system.hasMeshGenerator(name);
  }

  /**
   * @returns The MeshGenerator with the name \p name.
   */
  const MeshGenerator & getMeshGenerator(const std::string & name) const
  {
    return _mesh_generator_system.getMeshGenerator(name);
  }

  /**
   * @returns The final mesh generated by the mesh generator system
   */
  std::unique_ptr<MeshBase> getMeshGeneratorMesh()
  {
    return _mesh_generator_system.getSavedMesh(_mesh_generator_system.mainMeshGeneratorName());
  }

  /**
   * @returns The names of all mesh generators
   *
   * See MeshGeneratorSystem::getMeshGeneratorNames()
   */
  std::vector<std::string> getMeshGeneratorNames() const
  {
    return _mesh_generator_system.getMeshGeneratorNames();
  }

  /**
   * Append a mesh generator that will act on the final mesh generator in the system
   *
   * @param type The type of MeshGenerator
   * @param name The name of the MeshGenerator
   * @param params The params used to construct the MeshGenerator
   *
   * See MeshGeneratorSystem::appendMeshGenerator()
   */
  const MeshGenerator &
  appendMeshGenerator(const std::string & type, const std::string & name, InputParameters params)
  {
    return _mesh_generator_system.appendMeshGenerator(type, name, params);
  }

  /**
   * Whether this app is constructing mesh generators
   *
   * This is virtual to allow MooseUnitApp to override it so that we can
   * construct MeshGenerators in unit tests
   */
  virtual bool constructingMeshGenerators() const;

  ///@{
  /**
   * Sets the restart/recover flags
   * @param state The state to set the flag to
   */
  void setRestart(bool value);
  void setRecover(bool value);
  ///@}

  /// Returns whether the Application is running in check input mode
  bool checkInput() const { return _check_input; }

  /// Returns whether FPE trapping is turned on (either because of debug or user requested)
  bool getFPTrapFlag() const { return _trap_fpe; }

  /**
   * Returns a Boolean indicating whether a RelationshipManater exists with the same name.
   */
  bool hasRelationshipManager(const std::string & name) const;

  /**
   * Transfers ownership of a RelationshipManager to the application for lifetime management.
   * The RelationshipManager will NOT be duplicately added if an equivalent RelationshipManager
   * is already active. In that case, it's possible that the object will be destroyed if the
   * reference count drops to zero.
   */
  bool addRelationshipManager(std::shared_ptr<RelationshipManager> relationship_manager);

  /// The file suffix for the checkpoint mesh
  static const std::string & checkpointSuffix();
  /// The file suffix for meta data (header and data)
  static std::filesystem::path metaDataFolderBase(const std::filesystem::path & folder_base,
                                                  const std::string & map_suffix);
  /// The file suffix for restartable data
  std::filesystem::path restartFolderBase(const std::filesystem::path & folder_base) const;

  /**
   * @return The hit node that is responsible for creating the current action that is running,
   * if any
   *
   * Can be used to link objects that are created by an action to the action that
   * created them in input
   */
  const hit::Node * getCurrentActionHitNode() const;

private:
  /**
   * Purge this relationship manager from meshes and DofMaps and finally from us. This method is
   * private because only this object knows when we should remove relationship managers: when we are
   * adding relationship managers to this object's storage, we perform an operator>= comparison
   * between our existing RMs and the RM we are trying to add. If any comparison returns true, we do
   * not add the new RM because the comparison indicates that we would gain no new coverage.
   * However, if no comparison return true, then we add the new RM and we turn the comparison
   * around! Consequently if our new RM is >= than any of our preexisting RMs, we remove those
   * preexisting RMs using this method
   */
  void removeRelationshipManager(std::shared_ptr<RelationshipManager> relationship_manager);

#ifdef LIBTORCH_ENABLED
  /**
   * Function to determine the device which should be used by libtorch on this
   * application. We use this function to decide what is available on different
   * builds.
   * @param device Enum to describe if a cpu or a gpu should be used.
   */
  torch::DeviceType determineLibtorchDeviceType(const MooseEnum & device) const;
#endif

public:
  /**
   * Attach the relationship managers of the given type
   * Note: Geometric relationship managers that are supposed to be attached late
   * will be attached when Algebraic are attached.
   */
  void attachRelationshipManagers(Moose::RelationshipManagerType rm_type,
                                  bool attach_geometric_rm_final = false);

  /**
   * Attach geometric relationship managers to the given \p MeshBase object. This API is designed to
   * work with \p MeshGenerators which are executed at the very beginning of a simulation. No
   * attempt will be made to add relationship managers to a displaced mesh, because it doesn't exist
   * yet.
   */
  void attachRelationshipManagers(MeshBase & mesh, MooseMesh & moose_mesh);

  /**
   * Retrieve the relationship managers
   */
  const std::vector<std::shared_ptr<RelationshipManager>> & getReleationshipManagers();

  /**
   * Returns the Relationship managers info suitable for printing.
   */
  std::vector<std::pair<std::string, std::string>> getRelationshipManagerInfo() const;

  /**
   * Return the app level ExecFlagEnum, this contains all the available flags for the app.
   */
  const ExecFlagEnum & getExecuteOnEnum() const { return _execute_flags; }

  /**
   * @return Whether or not this app currently has an "initial" backup
   *
   * See _initial_backup and restoreFromInitialBackup() for more info.
   */
  bool hasInitialBackup() const
  {
    return _initial_backup != nullptr && *_initial_backup != nullptr;
  }

  /**
   * Whether to enable automatic scaling by default
   */
  bool defaultAutomaticScaling() const { return _automatic_automatic_scaling; }

  // Return the communicator for this application
  const std::shared_ptr<libMesh::Parallel::Communicator> getCommunicator() const { return _comm; }

  /**
   * Return the container of relationship managers
   */
  const std::set<std::shared_ptr<RelationshipManager>> & relationshipManagers() const
  {
    return _relationship_managers;
  }

  /**
   * Function to check the integrity of the restartable meta data structure
   */
  void checkMetaDataIntegrity() const;

  ///@{
  /**
   * Iterator based access to the extra RestartableDataMap objects; see Checkpoint.C for use case.
   *
   * These are MOOSE internal functions and should not be used otherwise.
   */
  auto getRestartableDataMapBegin() { return _restartable_meta_data.begin(); }

  auto getRestartableDataMapEnd() { return _restartable_meta_data.end(); }
  ///@}

  /**
   * Whether this application should by default error on Jacobian nonzero reallocations. The
   * application level setting can always be overridden by setting the \p
   * error_on_jacobian_nonzero_reallocation parameter in the \p Problem block of the input file
   */
  virtual bool errorOnJacobianNonzeroReallocation() const { return false; }

  /**
   * Registers an interface object for accessing with getInterfaceObjects.
   *
   * This should be called within the constructor of the interface in interest.
   */
  template <class T>
  void registerInterfaceObject(T & interface);

  /**
   * Gets the registered interface objects for a given interface.
   *
   * For this to work, the interface must register itself using registerInterfaceObject.
   */
  template <class T>
  const std::vector<T *> & getInterfaceObjects() const;

  static void addAppParam(InputParameters & params);
  static void addInputParam(InputParameters & params);

protected:
  /**
   * Helper method for dynamic loading of objects
   */
  void dynamicRegistration(const libMesh::Parameters & params);

  /**
   * Recursively loads libraries and dependencies in the proper order to fully register a
   * MOOSE application that may have several dependencies. REQUIRES: dynamic linking loader support.
   */
  void loadLibraryAndDependencies(const std::string & library_filename,
                                  const libMesh::Parameters & params,
                                  bool load_dependencies = true);

  /// Constructor is protected so that this object is constructed through the AppFactory object
  MooseApp(InputParameters parameters);

  /**
   * NOTE: This is an internal function meant for MOOSE use only!
   *
   * Register a piece of restartable data that will be used in a filter in/out during
   * deserialization. Note however that this data will always be written to the restart file.
   *
   * @param name The full (unique) name.
   * @param filter The filter name where to direct the name
   */
  void registerRestartableNameWithFilter(const std::string & name,
                                         Moose::RESTARTABLE_FILTER filter);

  /**
   * Runs post-initialization error checking that cannot be run correctly unless the simulation
   * has been fully set up and initialized.
   */
  void errorCheck();

  /// Parameters of this object
  InputParameters _pars;

  /// The string representation of the type of this object as registered (see registerApp(AppName))
  const std::string _type;

  /// The MPI communicator this App is going to use
  const std::shared_ptr<libMesh::Parallel::Communicator> _comm;

  /// The output file basename
  std::string _output_file_base;

  /// Whether or not file base is set through input or setOutputFileBase by MultiApp
  bool _file_base_set_by_user;

  /// Whether or not an output position has been set for this app
  bool _output_position_set;

  /// The output position
  Point _output_position;

  /// Whether or not an start time has been set
  bool _start_time_set;

  /// The time at which to start the simulation
  Real _start_time;

  /// Offset of the local App time to the "global" problem time
  Real _global_time_offset;

  /// Command line object
  std::shared_ptr<CommandLine> _command_line;

  /// Syntax of the input file
  Syntax _syntax;

  /// Input parameter storage structure; unique_ptr so we can control
  /// its destruction order
  std::unique_ptr<InputParameterWarehouse> _input_parameter_warehouse;

  /// The Factory responsible for building Actions
  ActionFactory _action_factory;

  /// Where built actions are stored
  ActionWarehouse _action_warehouse;

  /// OutputWarehouse object for this App
  OutputWarehouse _output_warehouse;

  /// Parser for parsing the input file
  const std::shared_ptr<Parser> _parser;

  /// Builder for building app related parser tree
  Moose::Builder _builder;

  /// Where the restartable data is held (indexed on tid)
  std::vector<RestartableDataMap> _restartable_data;

  /**
   * Data names that will only be read from the restart file during RECOVERY.
   * e.g. these names are _excluded_ during restart.
   */
  DataNames _recoverable_data_names;

  /// The PerfGraph object for this application (recoverable)
  PerfGraph & _perf_graph;

  /// The SolutionInvalidity object for this application
  SolutionInvalidity & _solution_invalidity;

  /// The RankMap is a useful object for determining how the processes are laid out on the physical hardware
  const RankMap _rank_map;

  /// Pointer to the executioner of this run (typically build by actions)
  std::shared_ptr<Executioner> _executioner;

  /// Pointer to the Executor of this run
  std::shared_ptr<Executor> _executor;

  /// Pointers to all of the Executors for this run
  std::map<std::string, std::shared_ptr<Executor>> _executors;

  /// Used in building the Executors
  /// Maps the name of the Executor block to the <type, params>
  std::unordered_map<std::string, std::pair<std::string, std::unique_ptr<InputParameters>>>
      _executor_params;

  /// Multiapp-related fixed point algorithm configuration details
  /// primarily intended  to be passed to and used by the executioner/executor system.
  FixedPointConfig _fixed_point_config;

  /// Indicates whether we are operating in the new/experimental executor mode
  /// instead of using the legacy executioner system.
  const bool _use_executor;

  /// Used to return an executor that does nothing
  std::shared_ptr<NullExecutor> _null_executor;

  /// Boolean to indicate whether to use a Nonlinear or EigenSystem (inspected by actions)
  bool _use_nonlinear;

  /// Boolean to indicate whether to use an eigenvalue executioner
  bool _use_eigen_value;

  /// System Information
  std::unique_ptr<SystemInfo> _sys_info;

  /// Indicates whether warnings, errors, or no output is displayed when unused parameters are detected
  enum UNUSED_CHECK
  {
    OFF,
    WARN_UNUSED,
    ERROR_UNUSED
  } _enable_unused_check;

  Factory _factory;

  /// Indicates whether warnings or errors are displayed when overridden parameters are detected
  bool _error_overridden;
  bool _ready_to_exit;
  /// The exit code
  int _exit_code;

  /// This variable indicates when a request has been made to restart from an Exodus file
  bool _initial_from_file;

  /// The Exodus reader when _initial_from_file is set to true
  std::shared_ptr<libMesh::ExodusII_IO> _ex_reader;

  /// This variable indicates that DistributedMesh should be used for the libMesh mesh underlying MooseMesh.
  bool _distributed_mesh_on_command_line;

  /// Whether or not this is a recovery run
  bool _recover;

  /// Whether or not this is a restart run
  bool _restart;

  /// Whether or not we are performing a split mesh operation (--split-mesh)
  bool _split_mesh;

  /// Whether or not we are using a (pre-)split mesh (automatically DistributedMesh)
  const bool _use_split;

  /// Whether or not FPE trapping should be turned on.
  bool _trap_fpe;

  /// The base name to restart/recover from.  If blank then we will find the newest checkpoint file.
  std::string _restart_recover_base;

  /// Whether or not this simulation should only run half its transient (useful for testing recovery)
  bool _test_checkpoint_half_transient;

  /// Map of outputer name and file number (used by MultiApps to propagate file numbers down through the multiapps)
  std::map<std::string, unsigned int> _output_file_numbers;

  /// true if we want to just check the input file
  bool _check_input;

  /// The relationship managers that have been added
  std::set<std::shared_ptr<RelationshipManager>> _relationship_managers;

  /// The relationship managers that have been attached (type -> RMs)
  std::map<Moose::RelationshipManagerType, std::set<const RelationshipManager *>>
      _attached_relationship_managers;

  /// A map from undisplaced relationship managers to their displaced clone (stored as the base
  /// GhostingFunctor). Anytime we clone in attachRelationshipManagers we create a map entry from
  /// the cloned undisplaced relationship manager to its displaced clone counterpart. We leverage
  /// this map when removing relationship managers/ghosting functors
  std::unordered_map<RelationshipManager *, std::shared_ptr<libMesh::GhostingFunctor>>
      _undisp_to_disp_rms;

  struct DynamicLibraryInfo
  {
    void * library_handle;
    std::string full_path;
    std::unordered_set<std::string> entry_symbols;
  };

  /// The library archive (name only), registration method and the handle to the method
  std::unordered_map<std::string, DynamicLibraryInfo> _lib_handles;

private:
  ///@{
  /// Structs that are used in the _interface_registry
  struct InterfaceRegistryObjectsBase
  {
    virtual ~InterfaceRegistryObjectsBase() {}
  };

  template <class T>
  struct InterfaceRegistryObjects : public InterfaceRegistryObjectsBase
  {
    std::vector<T *> _objects;
  };
  ///@}

  /** Method for creating the minimum required actions for an application (no input file)
   *
   * Mimics the following input file:
   *
   * [Mesh]
   *   type = GeneratedMesh
   *   dim = 1
   *   nx = 1
   * []
   *
   * [Executioner]
   *   type = Transient
   *   num_steps = 1
   *   dt = 1
   * []
   *
   * [Problem]
   *   solve = false
   * []
   *
   * [Outputs]
   *   console = false
   * []
   */
  void createMinimalApp();

  /**
   * Set a flag so that the parser will either warn or error when unused variables are seen after
   * parsing is complete.
   */
  void setCheckUnusedFlag(bool warn_is_error = false);

  /**
   * @return whether we have created any clones for the provided template relationship manager and
   * mesh yet. This may be false for instance when we are in the initial add relationship manager
   * stage and haven't attempted attaching any relationship managers to the mesh or dof map yet
   * (which is when we generate the clones). It's also maybe possible that we've created a clone of
   * a given \p template_rm but not for the provided mesh so we return false in that case as well
   */
  bool hasRMClone(const RelationshipManager & template_rm, const MeshBase & mesh) const;

  /**
   * Return the relationship manager clone originally created from the provided template
   * relationship manager and mesh
   */
  RelationshipManager & getRMClone(const RelationshipManager & template_rm,
                                   const MeshBase & mesh) const;

  /**
   * Take an input relationship manager, clone it, and then initialize it with provided mesh and
   * optional \p dof_map
   * @param template_rm The relationship manager template from which we will clone
   * @param moose_mesh The moose mesh to use for initialization
   * @param mesh The mesh to use for initialization
   * @param dof_map An optional parameter that, if provided, will be used to help init the cloned
   * relationship manager
   * @return a reference to the cloned and initialized relationship manager
   */
  RelationshipManager & createRMFromTemplateAndInit(const RelationshipManager & template_rm,
                                                    MooseMesh & moose_mesh,
                                                    MeshBase & mesh,
                                                    const libMesh::DofMap * dof_map = nullptr);

  /**
   * Creates a recoverable PerfGraph.
   *
   * This is a separate method so that it can be used in the constructor (multiple calls
   * are required to declare it).
   */
  PerfGraph & createRecoverablePerfGraph();

  /**
   * Creates a recoverable SolutionInvalidity.
   *
   * This is a separate method so that it can be used in the constructor (multiple calls
   * are required to declare it).
   */
  SolutionInvalidity & createRecoverableSolutionInvalidity();

  /**
   * Prints a message showing the installable inputs for a given application (if
   * getInstallableInputs has been overridden for an application).
   */
  bool showInputs() const;

  /**
   * Method to retrieve the installable inputs from a given applications <app>Revision.h file.
   */
  virtual std::string getInstallableInputs() const;

  /**
   * Handles the copy_inputs input parameter logic: Checks to see whether the passed argument is
   * valid (a readable installed directory) and recursively copies those files into a
   * read/writable location for the user.
   * @return a Boolean value used to indicate whether the application should exit early
   */
  bool copyInputs();

  /**
   * Handles the run input parameter logic: Checks to see whether a directory exists in user space
   * and launches the TestHarness to process the given directory.
   * @return a Boolean value used to indicate whether the application should exit early
   */
  bool runInputs();

  /// General storage for custom RestartableData that can be added to from outside applications
  std::unordered_map<RestartableDataMapName, std::pair<RestartableDataMap, std::string>>
      _restartable_meta_data;

  /// Enumeration for holding the valid types of dynamic registrations allowed
  enum RegistrationType
  {
    APPLICATION,
    REGALL
  };

  /// The combined warehouse for storing any MooseObject based object
  std::unique_ptr<TheWarehouse> _the_warehouse;

  /// Level of multiapp, the master is level 0. This used by the Console to indent output
  unsigned int _multiapp_level;

  /// Numbering in all the sub-apps on the same level
  unsigned int _multiapp_number;

  /// The mesh from master app
  const MooseMesh * const _master_mesh;

  /// The displaced mesh from master app
  const MooseMesh * const _master_displaced_mesh;

  /// The system that manages the MeshGenerators
  MeshGeneratorSystem _mesh_generator_system;

  /// The system that manages the ChainControls
  ChainControlDataSystem _chain_control_system;

  RestartableDataReader _rd_reader;

  /**
   * Execution flags for this App. Note: These are copied on purpose instead of maintaining a
   * reference to the ExecFlagRegistry registry. In the Multiapp case, the registry may be
   * augmented, changing the flags "known" to the application in the middle of executing the setup.
   * This causes issues with the application having to process flags that aren't specifically
   * registered.
   */
  const ExecFlagEnum _execute_flags;

  /// Cache output buffer so the language server can turn it off then back on
  std::streambuf * _output_buffer_cache;

  /// Whether to turn on automatic scaling by default
  const bool _automatic_automatic_scaling;

  /// CPU profiling
  bool _cpu_profiling = false;

  /// Memory profiling
  bool _heap_profiling = false;

  /// Map from a template relationship manager to a map in which the key-value pairs represent the \p
  /// MeshBase object and the clone of the template relationship manager, e.g. the top-level map key
  std::map<const RelationshipManager *,
           std::map<const MeshBase *, std::unique_ptr<RelationshipManager>>>
      _template_to_clones;

  /// Registration for interface objects
  std::map<std::type_index, std::unique_ptr<InterfaceRegistryObjectsBase>> _interface_registry;

  /// The backup for use in initial setup; this will get set from the _initial_backup
  /// input parameter that typically gets set from a MultiApp that has a backup
  /// This is a pointer to a pointer because at the time of construction of the app,
  /// the backup will not be filled yet.
  std::unique_ptr<Backup> * const _initial_backup;

#ifdef LIBTORCH_ENABLED
  /// The libtorch device this app is using.
  const torch::DeviceType _libtorch_device;
#endif

  // Allow FEProblemBase to set the recover/restart state, so make it a friend
  friend class FEProblemBase;
  friend class Restartable;
  friend class SubProblem;
};

template <typename T>
const T &
MooseApp::getParam(const std::string & name)
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

template <typename T>
const T &
MooseApp::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0), this);
}

template <typename T>
const T &
MooseApp::getRenamedParam(const std::string & old_name, const std::string & new_name) const
{
  // this enables having a default on the new parameter but bypassing it with the old one
  // Most important: accept new parameter
  if (isParamSetByUser(new_name) && !isParamValid(old_name))
    return InputParameters::getParamHelper(new_name, _pars, static_cast<T *>(0), this);
  // Second most: accept old parameter
  else if (isParamValid(old_name) && !isParamSetByUser(new_name))
    return InputParameters::getParamHelper(old_name, _pars, static_cast<T *>(0), this);
  // Third most: accept default for new parameter
  else if (isParamValid(new_name) && !isParamValid(old_name))
    return InputParameters::getParamHelper(new_name, _pars, static_cast<T *>(0), this);
  // Refuse: no default, no value passed
  else if (!isParamValid(old_name) && !isParamValid(new_name))
    mooseError(_pars.blockFullpath() + ": parameter '" + new_name +
               "' is being retrieved without being set.\n"
               "Did you mispell it?");
  // Refuse: both old and new parameters set by user
  else
    mooseError(_pars.blockFullpath() + ": parameter '" + new_name +
               "' may not be provided alongside former parameter '" + old_name + "'");
}

template <class T>
void
MooseApp::registerInterfaceObject(T & interface)
{
  static_assert(!std::is_base_of<MooseObject, T>::value, "T is not an interface");

  InterfaceRegistryObjects<T> * registry = nullptr;
  auto it = _interface_registry.find(typeid(T));
  if (it == _interface_registry.end())
  {
    auto new_registry = std::make_unique<InterfaceRegistryObjects<T>>();
    registry = new_registry.get();
    _interface_registry.emplace(typeid(T), std::move(new_registry));
  }
  else
    registry = static_cast<InterfaceRegistryObjects<T> *>(it->second.get());

  mooseAssert(std::count(registry->_objects.begin(), registry->_objects.end(), &interface) == 0,
              "Interface already registered");
  registry->_objects.push_back(&interface);
}

template <class T>
const std::vector<T *> &
MooseApp::getInterfaceObjects() const
{
  static_assert(!std::is_base_of<MooseObject, T>::value, "T is not an interface");

  const auto it = _interface_registry.find(typeid(T));
  if (it != _interface_registry.end())
    return static_cast<InterfaceRegistryObjects<T> *>(it->second.get())->_objects;
  const static std::vector<T *> empty;
  return empty;
}
