//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_ENABLE_AMR

#include "Moose.h"
#include "MooseError.h"
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "PerfGraphInterface.h"

#include "libmesh/parallel_object.h"

// libMesh
#include "libmesh/mesh_refinement.h"

class FEProblemBase;
class MooseMesh;
class DisplacedProblem;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<libMesh::VectorValue<Real>> VectorMooseVariable;
class MooseEnum;
class MultiMooseEnum;

// Forward declare classes in libMesh
namespace libMesh
{
class SystemNorm;
class ErrorVector;
class ErrorEstimator;
}

/**
 * Takes care of everything related to mesh adaptivity
 *
 */
class Adaptivity : public ConsoleStreamInterface,
                   public PerfGraphInterface,
                   public libMesh::ParallelObject
{
public:
  Adaptivity(FEProblemBase & fe_problem);
  virtual ~Adaptivity();

  /**
   * Initialize and turn on adaptivity for the simulation. initial_steps specifies the number of
   * adaptivity cycles to perform before the simulation starts and steps indicates the
   * number of adaptivity cycles to run during a steady (not transient) solve.  steps is not used
   * for transient or eigen solves.
   */
  void init(unsigned int steps, unsigned int initial_steps);

  /**
   * Set adaptivity parameter
   *
   * @param param_name the name of the parameter
   * @param param_value the value of parameter
   */
  template <typename T>
  void setParam(const std::string & param_name, const T & param_value);

  /**
   * Set the error estimator
   *
   * @param error_estimator_name the name of the error estimator (currently: Laplacian, Kelly, and
   * PatchRecovery)
   */
  void setErrorEstimator(const MooseEnum & error_estimator_name);

  /**
   * Set the error norm (FIXME: improve description)
   */
  void setErrorNorm(libMesh::SystemNorm & sys_norm);

  /**
   *
   */
  void setPrintMeshChanged(bool state = true) { _print_mesh_changed = state; }

  /**
   * Pull out the number of initial steps previously set by calling init()
   *
   * @return the number of initial steps
   */
  unsigned int getInitialSteps() const { return _initial_steps; }

  /**
   * Pull out the number of steps previously set by calling init()
   *
   * @return the number of steps
   */
  unsigned int getSteps() const { return _steps; }

  /**
   * Pull out the number of cycles_per_step previously set through the AdaptivityAction
   *
   * @return the number of cycles per step
   */
  unsigned int getCyclesPerStep() const { return _cycles_per_step; }

  /**
   * Set the number of cycles_per_step
   * @param num The number of cycles per step to execute
   */
  void setCyclesPerStep(const unsigned int & num) { _cycles_per_step = num; }

  /**
   * Pull out the _recompute_markers_during_cycles flag previously set through the AdaptivityAction
   *
   * @return the flag to recompute markers during adaptivity cycles
   */
  bool getRecomputeMarkersFlag() const { return _recompute_markers_during_cycles; }

  /**
   * Set the flag to recompute markers during adaptivity cycles
   * @param flag The flag to recompute markers
   */
  void setRecomputeMarkersFlag(const bool flag) { _recompute_markers_during_cycles = flag; }

  /**
   * Indicate whether the kind of adaptivity we're doing is p-refinement
   * @param doing_p_refinement Whether we're doing p-refinement
   * @param disable_p_refinement_for_families Families to disable p-refinement for
   */
  void doingPRefinement(bool doing_p_refinement,
                        const MultiMooseEnum & disable_p_refinement_for_families);

  /**
   * Adapts the mesh based on the error estimator used
   *
   * @return a boolean that indicates whether the mesh was changed
   */
  bool adaptMesh(std::string marker_name = std::string());

  /**
   * Used during initial adaptivity.
   *
   * @return a boolean that indicates whether the mesh was changed
   */
  bool initialAdaptMesh();

  /**
   * Performs uniform refinement of the passed Mesh object. The
   * number of levels of refinement performed is stored in the
   * MooseMesh object. No solution projection is performed in this
   * version.
   */
  static void uniformRefine(MooseMesh * mesh, unsigned int level = libMesh::invalid_uint);

  /**
   * Performs uniform refinement on the meshes in the current
   * object. Projections are performed of the solution vectors.
   */
  void uniformRefineWithProjection();

  /**
   * Allow adaptivity to be toggled programatically.
   * @param state The adaptivity state (on/off).
   */
  void setAdaptivityOn(bool state);

  /**
   * Is adaptivity on?
   *
   * @return true if mesh adaptivity is on, otherwise false
   */
  bool isOn() { return _mesh_refinement_on; }

  /**
   * Returns whether or not Adaptivity::init() has ran. Can
   * be used to indicate if mesh adaptivity is available.
   *
   * @return true if the Adaptivity system is ready to be used, otherwise false
   */
  bool isInitialized() { return _initialized; }

  /**
   * Sets the time when the adaptivity is active
   * @param start_time The time when adaptivity starts
   * @param stop_time The time when adaptivity stops
   */
  void setTimeActive(Real start_time, Real stop_time);

  /**
   * Tells this object we're using the "new" adaptivity system.
   */
  void setUseNewSystem();

  /**
   * Sets the name of the field variable to actually use to flag elements for refinement /
   * coarsening.
   * This must be a CONSTANT, MONOMIAL Auxiliary Variable Name that contains values
   * corresponding to libMesh::Elem::RefinementState.
   *
   * @param marker_field The name of the field to use for refinement / coarsening.
   */
  void setMarkerVariableName(std::string marker_field);

  /**
   * Sets the name of the field variable to actually use to flag elements for initial refinement /
   * coarsening.
   * This must be a CONSTANT, MONOMIAL Auxiliary Variable Name that contains values
   * corresponding to libMesh::Elem::RefinementState.
   *
   * @param marker_field The name of the field to use for refinement / coarsening.
   */
  void setInitialMarkerVariableName(std::string marker_field);

  /**
   * Set the maximum refinement level (for the new Adaptivity system).
   */
  void setMaxHLevel(unsigned int level) { _max_h_level = level; }

  /**
   * Return the maximum h-level.
   */
  unsigned int getMaxHLevel() { return _max_h_level; }

  /**
   * Set the interval (number of timesteps) between refinement steps.
   */
  void setInterval(unsigned int interval) { _interval = interval; }

  /**
   * Get an ErrorVector that will be filled up with values corresponding to the
   * indicator field name passed in.
   *
   * Note that this returns a reference... and the return value should be stored as a reference!
   *
   * @param indicator_field The name of the field to get an ErrorVector for.
   */
  libMesh::ErrorVector & getErrorVector(const std::string & indicator_field);

  /**
   * Update the ErrorVectors that have been requested through calls to getErrorVector().
   */
  void updateErrorVectors();

  /**
   * Query if an adaptivity step should be performed at the current time / time step
   */
  bool isAdaptivityDue();

protected:
  FEProblemBase & _fe_problem;
  MooseMesh & _mesh;

  /// on/off flag reporting if the adaptivity is being used
  bool _mesh_refinement_on;
  /// on/off flag reporting if the adaptivity system has been initialized
  bool _initialized;
  /// A mesh refinement object to be used either with initial refinement or with Adaptivity.
  std::unique_ptr<libMesh::MeshRefinement> _mesh_refinement;
  /// Error estimator to be used by the apps.
  std::unique_ptr<libMesh::ErrorEstimator> _error_estimator;
  /// Error vector for use with the error estimator.
  std::unique_ptr<libMesh::ErrorVector> _error;

  std::shared_ptr<DisplacedProblem> _displaced_problem;

  /// A mesh refinement object for displaced mesh
  std::unique_ptr<libMesh::MeshRefinement> _displaced_mesh_refinement;

  /// the number of adaptivity steps to do at the beginning of simulation
  unsigned int _initial_steps;
  /// steps of adaptivity to perform
  unsigned int _steps;

  /// True if we want to print out info when mesh has changed
  bool _print_mesh_changed;

  /// Time
  Real & _t;
  /// Time Step
  int & _step;
  /// intreval between adaptivity runs
  unsigned int _interval;
  /// When adaptivity start
  Real _start_time;
  /// When adaptivity stops
  Real _stop_time;
  /// The number of adaptivity cycles per step
  unsigned int _cycles_per_step;

  /// Whether or not to use the "new" adaptivity system
  bool _use_new_system;

  /// Name of the marker variable if using the new adaptivity system
  std::string _marker_variable_name;

  /// Name of the initial marker variable if using the new adaptivity system
  std::string _initial_marker_variable_name;

  /// The maximum number of refinement levels
  unsigned int _max_h_level;

  /// Whether or not to recompute markers during adaptivity cycles
  bool _recompute_markers_during_cycles;

  /// Stores pointers to ErrorVectors associated with indicator field names
  std::map<std::string, std::unique_ptr<libMesh::ErrorVector>> _indicator_field_to_error_vector;

  bool _p_refinement_flag = false;
};

template <typename T>
void
Adaptivity::setParam(const std::string & param_name, const T & param_value)
{
  if (param_name == "refine fraction")
  {
    _mesh_refinement->refine_fraction() = param_value;
    if (_displaced_mesh_refinement)
      _displaced_mesh_refinement->refine_fraction() = param_value;
  }
  else if (param_name == "coarsen fraction")
  {
    _mesh_refinement->coarsen_fraction() = param_value;
    if (_displaced_mesh_refinement)
      _displaced_mesh_refinement->coarsen_fraction() = param_value;
  }
  else if (param_name == "max h-level")
  {
    _mesh_refinement->max_h_level() = param_value;
    if (_displaced_mesh_refinement)
      _displaced_mesh_refinement->max_h_level() = param_value;
  }
  else if (param_name == "cycles_per_step")
    _cycles_per_step = param_value;
  else if (param_name == "recompute_markers_during_cycles")
    _recompute_markers_during_cycles = param_value;
  else
    mooseError("Invalid Param in adaptivity object");
}
#endif // LIBMESH_ENABLE_AMR
