#ifndef ADAPTIVITY_H
#define ADAPTIVITY_H

#include <string>

// libMesh
#include "system_norm.h"
#include "mesh_refinement.h"
#include "error_vector.h"
#include "error_estimator.h"

class SubProblem;
class MooseMesh;

class Adaptivity
{
public:
  Adaptivity(SubProblem & subproblem);
  virtual ~Adaptivity();

  void init(unsigned int steps, unsigned int initial_steps);
  template<typename T>
  void setParam(const std::string & param_name, const T & param_value);
  void setErrorEstimator(const std::string &error_estimator_name);
  void setErrorNorm(SystemNorm &sys_norm);

  void adaptMesh();

  bool isOn() { return _mesh_refinement_on; }

protected:
  SubProblem & _subproblem;
  MooseMesh & _mesh;

  bool _mesh_refinement_on;

  /**
   * A mesh refinement object to be used either with initial refinement or with Adaptivity.
   */
  MeshRefinement * _mesh_refinement;

  /**
   * Error estimator to be used by the apps.
   */
  ErrorEstimator * _error_estimator;

  /**
   * Error vector for use with the error estimator.
   */
  ErrorVector * _error;

};

/**
 * Set parameters for adaptivity
 */
template<typename T>
void
Adaptivity::setParam(const std::string &param_name, const T &param_value)
{
  if (param_name == "refine fraction")
  {
    _mesh_refinement->refine_fraction() = param_value;
  }
  else if (param_name == "coarsen fraction")
  {
    _mesh_refinement->coarsen_fraction() = param_value;
  }
  else if (param_name == "max h-level")
  {
    _mesh_refinement->max_h_level() = param_value;
  }
  else
  {
    // TODO: spit out some warning/error
  }
}

#endif /* ADAPTIVITY_H */
