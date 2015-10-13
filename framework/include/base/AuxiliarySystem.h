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

#ifndef AUXILIARYSYSTEM_H
#define AUXILIARYSYSTEM_H

#include <set>
#include "SystemBase.h"
#include "ExecStore.h"
#include "AuxWarehouse.h"
#include "TimeIntegrator.h"

// libMesh include
#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"
#include "libmesh/transient_system.h"

#include "libmesh/numeric_vector.h"

class AuxKernel;
class FEProblem;

/**
 * A system that holds auxiliary variables
 *
 */
class AuxiliarySystem : public SystemTempl<TransientExplicitSystem>
{
public:
  AuxiliarySystem(FEProblem & subproblem, const std::string & name);
  virtual ~AuxiliarySystem();

  virtual void init();

  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void residualSetup();
  virtual void jacobianSetup();

  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL);

  /**
   * Add a time integrator
   * @param type Type of the integrator
   * @param name The name of the integrator
   * @param parameters Integrator params
   */
  void addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters);

  /**
   * Adds an auxiliary kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Parameters for this kernel
   */
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a scalar kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);

  virtual const NumericVector<Number> * & currentSolution() { _current_solution = _sys.current_local_solution.get(); return _current_solution; }

  virtual NumericVector<Number> & solutionUDot();

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution();

  // This is an empty function since the Aux system doesn't have a matrix!
  virtual void augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                               std::vector<dof_id_type> & /*n_nz*/,
                               std::vector<dof_id_type> & /*n_oz*/);

  /**
   * Compute auxiliary variables
   * @param type Time flag of which variables should be computed
   */
  virtual void compute(ExecFlagType type);

  /**
   * Get a list of dependent UserObjects for this exec type
   * @param type Execution flag type
   * @return a set of dependent user objects
   */
  std::set<std::string> getDependObjects(ExecFlagType type);

  /**
   * Adds a solution length vector to the system.
   *
   * @param vector_name The name of the vector.
   * @param project Whether or not to project this vector when doing mesh refinement.
   *                If the vector is just going to be recomputed then there is no need to project it.
   * @param type What type of parallel vector.  This is usually either PARALLEL or GHOSTED.
   *             GHOSTED is needed if you are going to be accessing off-processor entries.
   *             The ghosting pattern is the same as the solution vector.
   */
  NumericVector<Number> & addVector(const std::string & vector_name, const bool project, const ParallelType type);

  /**
   * Get the minimum quadrature order for evaluating elemental auxiliary variables
   */
  virtual Order getMinQuadratureOrder();

  /**
   * Indicated whether this system needs material properties on boundaries.
   * @return Boolean if IntegratedBCs are active
   */
  bool needMaterialOnSide(BoundaryID bnd_id);

protected:
  void computeScalarVars(ExecFlagType type);
  void computeNodalVars(ExecFlagType type);
  void computeElementalVars(ExecFlagType type);

  FEProblem & _fe_problem;

  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;
  /// Serialized version of the solution vector
  NumericVector<Number> & _serialized_solution;
  /// Time integrator
  MooseSharedPointer<TimeIntegrator> _time_integrator;
  /// solution vector for u^dot
  NumericVector<Number> & _u_dot;

  /// Whether or not a copy of the residual needs to be made
  bool _need_serialized_solution;

  // Variables
  std::vector<std::map<std::string, MooseVariable *> > _nodal_vars;
  std::vector<std::map<std::string, MooseVariable *> > _elem_vars;

  ExecStore<AuxWarehouse> _auxs;

  friend class AuxKernel;
  friend class ComputeNodalAuxVarsThread;
  friend class ComputeNodalAuxBcsThread;
  friend class ComputeElemAuxVarsThread;
  friend class ComputeElemAuxBcsThread;
  friend class ComputeIndicatorThread;
  friend class ComputeMarkerThread;
  friend class FlagElementsThread;
  friend class ComputeNodalKernelsThread;
};

#endif /* EXPLICITSYSTEM_H */
