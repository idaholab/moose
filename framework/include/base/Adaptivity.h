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

#ifndef ADAPTIVITY_H
#define ADAPTIVITY_H

#include "Moose.h"

#ifdef LIBMESH_ENABLE_AMR

#include <string>

// libMesh
#include "system_norm.h"
#include "mesh_refinement.h"
#include "error_vector.h"
#include "error_estimator.h"

class FEProblem;
class MooseMesh;
class DisplacedProblem;

/**
 * Takes care of everything related to mesh adaptivity
 *
 */
class Adaptivity
{
public:
  Adaptivity(FEProblem & subproblem);
  virtual ~Adaptivity();

  /**
   * Initialize the initial adaptivity ;-)
   *
   * @param steps TODO: describe me
   * @param initial_steps number of steps to do in the initial adaptivity
   */
  void init(unsigned int steps, unsigned int initial_steps);

  /**
   * Set adaptivity parameter
   *
   * @param param_name the name of the parameter
   * @param param_value the value of parameter
   */
  template<typename T>
  void setParam(const std::string & param_name, const T & param_value);

  /**
   * Set the error estimator
   *
   * @param error_estimator_name the name of the error estimator (currently: Laplacian and Kelly)
   */
  void setErrorEstimator(const std::string & error_estimator_name);

  /**
   * Set the error norm (FIXME: improve description)
   */
  void setErrorNorm(SystemNorm &sys_norm);

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
   * Adapts the mesh based on the error estimator used
   */
  void adaptMesh();

  /**
   * Does 'level' levels of uniform refinements
   */
  void uniformRefine(unsigned int level);

  /**
   * Is adaptivity on?
   *
   * @return true if we do mesh adaptivity, otherwise false
   */
  bool isOn() { return _mesh_refinement_on; }

  /**
   * Sets the time when the adaptivity is active
   * @param start_time The time when adaptivity starts
   * @param stop_time The time when adaptivity stops
   */
  void setTimeActive(Real start_time, Real stop_time);

protected:
  FEProblem & _subproblem;
  MooseMesh & _mesh;

  bool _mesh_refinement_on;                     ///< on/off flag reporting if the adaptivity is being used
  MeshRefinement * _mesh_refinement;            ///< A mesh refinement object to be used either with initial refinement or with Adaptivity.
  ErrorEstimator * _error_estimator;            ///< Error estimator to be used by the apps.
  ErrorVector * _error;                         ///< Error vector for use with the error estimator.

  DisplacedProblem * & _displaced_problem;
  MeshRefinement * _displaced_mesh_refinement;  ///< A mesh refinement object for displaced mesh

  unsigned int _initial_steps;                  ///< the number of adaptivity steps to do at the beginning of simulation
  unsigned int _steps;                          ///< steps of adaptivity to perform

  bool _print_mesh_changed;                     ///< True if we want to print out info when mesh has changed

  Real & _t;                                    ///< Time
  Real _start_time;                             ///< When adaptivity start
  Real _stop_time;                              ///< When adaptivity stops
};

template<typename T>
void
Adaptivity::setParam(const std::string &param_name, const T &param_value)
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
  else
  {
    // TODO: spit out some warning/error
  }
}
#endif //LIBMESH_ENABLE_AMR

#endif /* ADAPTIVITY_H */
