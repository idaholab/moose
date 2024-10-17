//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <chrono>

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "SetupInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "ReporterInterface.h"
#include "AdvancedOutputUtils.h"
#include "PerfGraphInterface.h"
#include "FunctionInterface.h"

class MooseMesh;
class Times;

// libMesh forward declarations
namespace libMesh
{
class EquationSystems;
}

/**
 * Based class for output objects
 *
 * Each output class (e.g., Exodus) should inherit from this base class. At a minimum, the pure
 * virtual methods for the various types of output must be defined in the child class.
 *
 * @see Exodus Console CSV
 */
class Output : public MooseObject,
               public Restartable,
               public MeshChangedInterface,
               public SetupInterface,
               public FunctionInterface,
               public PostprocessorInterface,
               public VectorPostprocessorInterface,
               public ReporterInterface,
               public PerfGraphInterface
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   *
   * @see initAvailable init separate
   */
  Output(const InputParameters & parameters);

  /**
   * Get the output time.
   * @return The output time, which may be different than the simulation time
   *
   * When the Executioner is steady this utilizes the time_step and when Transient the actual time
   * is used.
   */
  virtual Real time();

  /**
   * Get the old output time.
   * @return The old output time, which may be different than the simulation time
   *
   * @see time()
   */
  virtual Real timeOld();

  /**
   * Get the current time step size
   */
  virtual Real dt();

  /**
   * Get old time step size
   */
  virtual Real dtOld();

  /**
   * Get the current time step
   */
  virtual int timeStep();

  /**
   * Get the output interval
   */
  const unsigned int & interval() const;

  /**
   * Get the current 'execute_on' selections for display
   */
  const MultiMooseEnum & executeOn() const;

  /**
   * Returns true if this object is an AdvancedOutput object
   */
  bool isAdvanced();

  /**
   * Returns the advanced 'execute_on' settings.
   *
   * Check if this is valid first with isAdvanced()
   */
  virtual const OutputOnWarehouse & advancedExecuteOn() const;

  /**
   * Return an ExecFlagEnum object with the available execution flags for Output objects.
   */
  static ExecFlagEnum getDefaultExecFlagEnum();

  /**
   * Method for controlling the allow output state
   * @param state The state to set the allow flag to
   */
  void allowOutput(bool state) { _allow_output = state; }

  /**
   * A static helper for injecting deprecated parameters
   */
  static void addDeprecatedInputParameters(InputParameters & params);

  /**
   * A single call to this function should output all the necessary data for a single timestep.
   * @param type The type execution flag (see Moose.h)
   *
   * @see outputNodalVariables outputElementalVariables outputScalarVariables outputPostprocessors
   */
  virtual void outputStep(const ExecFlagType & type);

  const std::set<Real> & getSyncTimes() { return _sync_times; }

protected:
  /**
   * Overload this function with the desired output activities
   */
  virtual void output() = 0;

  /**
   * A method called just prior to the solve, this is used by PetscOutput to perform the necessary
   * setup actions for each timestep
   */
  virtual void solveSetup();

  /**
   * Handles logic for determining if a step should be output
   * @return True if a call if output should be performed
   */
  virtual bool shouldOutput();

  /**
   * Returns true if the output interval is satisfied
   * \todo{Implement additional types of intervals (e.g., simulation time and real time)}
   */
  virtual bool onInterval();

  /**
   * Function to set the wall time interval based on value of command line parameter (used for
   * testing only).
   * @param cli_param_name The name of the command line parameter to set the wall time interval to
   *
   */
  void setWallTimeIntervalFromCommandLineParam();

  /// Pointer the the FEProblemBase object for output object (use this)
  FEProblemBase * _problem_ptr;

  /// Transient flag (true = transient)
  bool _transient;

  /// Flag for using displaced mesh
  bool _use_displaced;

  /// Reference the the libMesh::EquationSystems object that contains the data
  libMesh::EquationSystems * _es_ptr;

  /// A convenience pointer to the current mesh (reference or displaced depending on "use_displaced")
  MooseMesh * _mesh_ptr;

  /// Flag for forcing call to outputSetup() with every call to output() (restartable)
  bool _sequence;

  /// The common Execution types; this is used as the default execution type for everything except system information and input
  ExecFlagEnum _execute_on;

  /**
   * Current execute on flag. This is different from the flag provided by
   * FEProblemBase::getCurrentExecuteOnFlag() const, as outputs are triggered
   * in PETSc callbacks which cannot update  FEProblemBase::_current_execute_on_flag
   * so we shadow it with a new member of the same name.
   */
  ExecFlagType _current_execute_flag;

  /// The current time for output purposes
  Real & _time;

  /// The old time
  Real & _time_old;

  /// The current time step
  int & _t_step;

  /// Time step delta
  Real & _dt;

  /// Old time step delta
  Real & _dt_old;

  /// The number of outputs written
  unsigned int _num;

  /// Whether time step interval is set by AddParam
  const bool _time_step_interval_set_by_addparam;

  /// The output time step interval
  unsigned int _time_step_interval;

  /// Minimum simulation time between outputs
  const Real _min_simulation_time_interval;

  /// Target simulation time between outputs
  const Real _simulation_time_interval;

  /// Target wall time between outputs in seconds
  Real _wall_time_interval;

  /// Sync times for this outputter
  std::set<Real> _sync_times;

  /// Sync times object for this outputter
  const Times * const _sync_times_object;

  /// Start outputting time
  Real _start_time;

  /// End outputting time
  Real _end_time;

  /// Start outputting at this time step
  int _start_step;

  /// End outputting at this time step
  int _end_step;

  /// Time checking tolerance
  Real _t_tol;

  /// Flag for only executing at sync times
  bool _sync_only;

  /// Flag for disabling output
  bool _allow_output;

  /// Flag for advanced output testing
  bool _is_advanced;

  /// Storage for the individual component execute flags
  // This is here rather than in AdvancedOutput to allow generic
  // access to this data from the Console object for displaying
  // the output settings.
  OutputOnWarehouse _advanced_execute_on;

  /// last simulation time an output has occured
  Real & _last_output_simulation_time;

  /// last wall time an output has occured
  std::chrono::time_point<std::chrono::steady_clock> _last_output_wall_time;

  /// time in seconds since last output
  Real _wall_time_since_last_output;

  friend class OutputWarehouse;
};
