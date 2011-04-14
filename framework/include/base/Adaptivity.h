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

#ifdef LIBMESH_ENABLE_AMR

#include <string>

// libMesh
#include "system_norm.h"
#include "mesh_refinement.h"
#include "error_vector.h"
#include "error_estimator.h"

class MProblem;
class MooseMesh;

class Adaptivity
{
public:
  Adaptivity(MProblem & subproblem);
  virtual ~Adaptivity();

  void init(unsigned int steps, unsigned int initial_steps);
  template<typename T>
  void setParam(const std::string & param_name, const T & param_value);
  void setErrorEstimator(const std::string &error_estimator_name);
  void setErrorNorm(SystemNorm &sys_norm);

  /**
   * Perform initial adaptivity steps
   *
   * FIXME: better name
   */
  void initial();

  unsigned int getSteps() const;

  void adaptMesh();

  bool isOn() { return _mesh_refinement_on; }

protected:
  MProblem & _subproblem;
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


  unsigned int _initial_steps;                          // the number of adaptivity steps to do at the beginning of simulation

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
#endif //LIBMESH_ENABLE_AMR

#endif /* ADAPTIVITY_H */
