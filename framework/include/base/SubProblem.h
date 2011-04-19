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

#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

#include "ParallelUniqueId.h"
#include "Problem.h"
#include "SubProblemInterface.h"

// libMesh include
#include "equation_systems.h"
#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"

class MooseMesh;

/**
 * Generic class for solving transient nonlinear problems
 *
 */
class SubProblem :
  public Problem,
  public SubProblemInterface
{
public:
  SubProblem(MooseMesh & mesh, Problem * parent = NULL);
  virtual ~SubProblem();

  virtual Problem * parent() { return _parent; }
  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }

  virtual void init();
  virtual void solve() = 0;
  virtual bool converged() = 0;

  /**
   *
   */
  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual void copySolutionsBackwards() = 0;

  virtual Real & time() { return _time; }
  virtual int & timeStep() { return _t_step; }
  virtual Real & dt() { return _dt; }
  virtual Real & dtOld() { return _dt_old; }

  virtual void transient(bool trans) { _transient = trans; }
  virtual bool transient() { return _transient; }

  virtual Order getQuadratureOrder() = 0;

  /// Will make sure that all dofs connected to elem_id are ghosted to this processor
  virtual void addGhostedElem(unsigned int elem_id) = 0;

  /// Will make sure that all necessary elements from boundary_id are ghosted to this processor
  virtual void addGhostedBoundary(unsigned int boundary_id) = 0;
  
protected:
  Problem * _parent;
  MooseMesh & _mesh;
  EquationSystems & _eq;

  bool _transient;
  Real & _time;
  int & _t_step;
  Real & _dt;
  Real _dt_old;
};


namespace Moose
{

void initial_condition(EquationSystems & es, const std::string & system_name);

} // namespace Moose


#endif /* SUBPROBLEM_H */
