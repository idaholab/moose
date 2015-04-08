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

#ifndef MOOSEAPP_H
#define MOOSEAPP_H

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "Moose.h"
#include "Parser.h"
#include "MooseSyntax.h"
#include "ActionWarehouse.h"
#include "SystemInfo.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "OutputWarehouse.h"

// libMesh includes
#include "libmesh/parallel_object.h"

class Executioner;
class MooseApp;
class RecoverBaseAction;

template<>
InputParameters validParams<MooseApp>();

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
class MooseApp : public libMesh::ParallelObject
{
public:
  virtual ~MooseApp();

  /**
   * Get the name of the object
   * @return The name of the object
   */
  const std::string & name() { return _name; }

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  InputParameters & parameters() { return _pars; }

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

  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  /**
   * Run the application
   */
  virtual void run();

  /**
   * Setup options based on InputParameters.
   */
  virtual void setupOptions();

  ActionWarehouse & actionWarehouse() { return _action_warehouse; }

  Parser & parser() { return _parser; }

  Syntax & syntax() { return _syntax; }

  /**
   * Set the input file name.
   */
  void setInputFileName(std::string input_file_name);

  /**
   * Returns the input file name that was set with setInputFileName
   */
  std::string getInputFileName(){ return _input_filename; }

  /**
   * Override the selection of the output file base name.
   */
  void setOutputFileBase(std::string output_file_base) { _output_file_base = output_file_base; }

  /**
   * Override the selection of the output file base name.
   */
  std::string getOutputFileBase();

  /**
   * Tell the app to output in a specific position.
   */
  void setOutputPosition(Point p);

  /**
   * Whether or not an output position has been set.
   * @return True if it has
   */
  bool hasOutputPosition() { return _output_position_set; }

  /**
   * Get the output position.
   * @return The position offset for the output.
   */
  Point getOutputPosition() { return _output_position; }

  /**
   * Set the starting time for the simulation.  This will override any choice
   * made in the input file.
   *
   * @param time The start time for the simulation.
   */
  void setStartTime(const Real time) { _start_time_set = true; _start_time = time; }

  /**
   * @return Whether or not a start time has been programmatically set using setStartTime()
   */
  bool hasStartTime() { return _start_time_set; }

  /**
   * @return The start time
   */
  Real getStartTime() { return _start_time; }

  /**
   * Each App has it's own local time.  The "global" time of the whole problem might be
   * different.  This offset is how far off the local App time is from the global time.
   */
  void setGlobalTimeOffset(const Real offset) { _global_time_offset = offset; }

  /**
   * Each App has it's own local time.  The "global" time of the whole problem might be
   * different.  This offset is how far off the local App time is from the global time.
   */
  Real getGlobalTimeOffset() { return _global_time_offset; }

  /**
   * Return the filename that was parsed
   */
  std::string getFileName(bool stripLeadingPath = true) const;

  /**
   * Set a flag so that the parser will either warn or error when unused variables are seen after
   * parsing is complete.
   */
  void setCheckUnusedFlag(bool warn_is_error = false);

  /**
   * Set a flag so that the parser will throw an error if overridden parameters are detected
   */
  void setErrorOverridden();

  /**
   * Removes warnings and error checks for unrecognized variables in the input file
   */
  void disableCheckUnusedFlag();

  /**
   * Tell MOOSE to compute all aux kernels when any user objects are computed - deprecated behavior
   */
  bool & legacyUoAuxComputationDefault();

  /**
   * Tell MOOSE to compute all aux kernels when any user objects are computed - deprecated behavior
   */
  bool & legacyUoInitializationDefault();

  /**
   * Retrieve the Executioner for this App.
   */
  Executioner * getExecutioner() { return _executioner.get(); }

  /**
   * Set a Boolean indicating whether this app will use a Nonlinear or Eigen System.
   */
  bool & useNonlinear() { return _use_nonlinear; }

  /**
   * Retrieve the Factory associated with this App.
   */
  Factory & getFactory() { return _factory; }

  /**
   * Retrieve the ActionFactory associated with this App.
   */
  ActionFactory & getActionFactory() { return _action_factory; }

  /**
   * Get the command line
   * @return The reference to the command line object
   * Setup options based on InputParameters.
   */
  MooseSharedPointer<CommandLine> commandLine() { return _command_line; }

  /**
   * This method is here so we can determine whether or not we need to
   * use a separate reader to read the mesh BEFORE we create the mesh.
   */
  bool & setFileRestart() { return _initial_from_file; }

  /**
   * Actually build everything in the input file.
   */
  virtual void runInputFile();

  /**
   * Execute the Executioner that was built.
   */
  virtual void executeExecutioner();

  /**
   * Returns true if the user specified --parallel-mesh on the command line and false
   * otherwise.
   */
  bool getParallelMeshOnCommandLine() const { return _parallel_mesh_on_command_line; }

  /**
   * Whether or not this is a "recover" calculation.
   */
  bool isRecovering() const { return _recover; }

  /**
   * Whether or not this is a "recover" calculation.
   */
  bool isRestarting() const { return _restart; }

  /**
   * Return true if the recovery file base is set
   */
  bool hasRecoverFileBase() { return !_recover_base.empty(); }

  /**
   * The file_base for the recovery file.
   */
  std::string getRecoverFileBase() { return _recover_base; }

  /**
   * mutator for recover_base (set by RecoverBaseAction)
   */
  void setRecoverFileBase(std::string recover_base) { _recover_base = recover_base; }

  /**
   *  Whether or not this simulation should only run half its transient (useful for testing recovery)
   */
  bool halfTransient() { return _half_transient; }

  /**
   * Store a map of outputter names and file numbers
   * The MultiApp system requires this to get the file numbering to propagate down through the Multiapps.
   * @param numbers Map of outputter names and file numbers
   *
   * @see MultiApp TransientMultiApp OutputWarehouse
   */
  void setOutputFileNumbers(std::map<std::string, unsigned int> numbers){ _output_file_numbers = numbers; }

  /**
   * Store a map of outputter names and file numbers
   * The MultiApp system requires this to get the file numbering to propogate down through the
   * multiapps.
   *
   * @see MultiApp TransientMultiApp
   */
  std::map<std::string, unsigned int> & getOutputFileNumbers(){ return _output_file_numbers; }

  /**
   * Return true if the output position has been set
   */
  bool hasOutputWarehouse(){ return _output_position_set; }

  /**
   * The OutputWarehouse for this App
   * @return Reference to the OutputWarehouse object
   */
  OutputWarehouse & getOutputWarehouse();

  /**
   * Set the OutputWarehouse object
   * The CoupledExecutioner requires multiple OutputWarehouses, this allows the warehouses
   * to be swapped out.
   *
   * If this function is called then getOutputWarehouse will return a reference to the
   * _alternate_output_warehouse rather than _output_warehouse.
   */
  void setOutputWarehouse(OutputWarehouse * owh){ _alternate_output_warehouse = owh; }

  /**
   * Get SystemInfo object
   * @return A pointer to the SystemInformation object
   */
  SystemInfo * getSystemInfo() { return _sys_info.get(); }

protected:

  MooseApp(const std::string & name, InputParameters parameters);

  virtual void meshOnly(std::string mesh_file_name);

  /// The name of this object
  std::string _name;

  /// Parameters of this object
  InputParameters _pars;

  /// The MPI communicator this App is going to use
  const MooseSharedPointer<Parallel::Communicator> _comm;

  /// Input file name used
  std::string _input_filename;

  /// The output file basename
  std::string _output_file_base;

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
  MooseSharedPointer<CommandLine> _command_line;

  /// Syntax of the input file
  Syntax _syntax;

  /// An alternate OutputWarehouse object (required for CoupledExecutioner)
  OutputWarehouse * _alternate_output_warehouse;

  /// OutputWarehouse object for this App
  OutputWarehouse * _output_warehouse;

  /// The Factory responsible for building Actions
  ActionFactory _action_factory;

  /// Where built actions are stored
  ActionWarehouse _action_warehouse;

  /// Parser for parsing the input file
  Parser _parser;

  /// Pointer to the executioner of this run (typically build by actions)
  MooseSharedPointer<Executioner> _executioner;

  /// Boolean to indicate whether to use a Nonlinear or EigenSystem (inspected by actions)
  bool _use_nonlinear;

  /// System Information
  MooseSharedPointer<SystemInfo> _sys_info;

  /// Indicates whether warnings, errors, or no output is displayed when unused parameters are detected
  enum UNUSED_CHECK { OFF, WARN_UNUSED, ERROR_UNUSED } _enable_unused_check;

  Factory _factory;

  /// Indicates whether warnings or errors are displayed when overridden parameters are detected
  bool _error_overridden;
  bool _ready_to_exit;

  /// This variable indicates when a request has been made to restart from an Exodus file
  bool _initial_from_file;

  /// This variable indicates that ParallelMesh should be used for the libMesh mesh underlying MooseMesh.
  bool _parallel_mesh_on_command_line;

  /// Whether or not this is a recovery run
  bool _recover;

  /// Whether or not this is a restart run
  bool _restart;

  /// The base name to recover from.  If blank then we will find the newest recovery file.
  std::string _recover_base;

  /// Whether or not this simulation should only run half its transient (useful for testing recovery)
  bool _half_transient;

  /// Map of outputer name and file number (used by MultiApps to propagate file numbers down through the multiapps)
  std::map<std::string, unsigned int> _output_file_numbers;

  /// Legacy Uo Aux computation flag
  bool _legacy_uo_aux_computation_default;

  /// Legacy Uo Initialization flag
  bool _legacy_uo_initialization_default;

  /// true if we want to just check the input file
  bool _check_input;

private:

  ///@{
  /**
   * Sets the restart/recover flags
   * @param state The state to set the flag to
   */
  void setRestart(const bool & value){ _restart = value; }
  void setRecover(const bool & value){ _recover = value; }
  ///@}

  // Allow FEProblem to set the recover/restart state, so make it a friend
  friend class FEProblem;
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
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

#endif /* MOOSEAPP_H */
