//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SystemBase.h"
#include "ExecuteMooseObjectWarehouse.h"
#include "PerfGraphInterface.h"

#include "libmesh/explicit_system.h"
#include "libmesh/transient_system.h"

// Forward declarations
template <typename ComputeValueType>
class AuxKernelTempl;
typedef AuxKernelTempl<Real> AuxKernel;
typedef AuxKernelTempl<RealVectorValue> VectorAuxKernel;
class FEProblemBase;
class TimeIntegrator;
class AuxScalarKernel;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * A system that holds auxiliary variables
 *
 */
class AuxiliarySystem : public SystemBase, public PerfGraphInterface
{
public:
  AuxiliarySystem(FEProblemBase & subproblem, const std::string & name);
  virtual ~AuxiliarySystem();

  virtual void init() override;
  virtual void addExtraVectors() override;

  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void subdomainSetup();
  virtual void residualSetup();
  virtual void jacobianSetup();
  virtual void updateActive(THREAD_ID tid);

  virtual void addVariable(const std::string & var_type,
                           const std::string & name,
                           InputParameters & parameters) override;
  /**
   * Add a time integrator
   * @param type Type of the integrator
   * @param name The name of the integrator
   * @param parameters Integrator params
   */
  void addTimeIntegrator(const std::string & type,
                         const std::string & name,
                         InputParameters & parameters) override;
  using SystemBase::addTimeIntegrator;

  /**
   * Adds u_dot, u_dotdot, u_dot_old and u_dotdot_old
   * vectors if requested by the time integrator
   */
  void addDotVectors();

  /**
   * Adds an auxiliary kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Parameters for this kernel
   */
  void addKernel(const std::string & kernel_name,
                 const std::string & name,
                 InputParameters & parameters);

  /**
   * Adds a scalar kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addScalarKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters & parameters);

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) override;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) override;

  const NumericVector<Number> * const & currentSolution() const override
  {
    _current_solution = _sys.current_local_solution.get();
    return _current_solution;
  }

  NumericVector<Number> * solutionUDot() override { return _u_dot; }
  NumericVector<Number> * solutionUDotDot() override { return _u_dotdot; }
  NumericVector<Number> * solutionUDotOld() override { return _u_dot_old; }
  NumericVector<Number> * solutionUDotDotOld() override { return _u_dotdot_old; }
  const NumericVector<Number> * solutionUDot() const override { return _u_dot; }
  const NumericVector<Number> * solutionUDotDot() const override { return _u_dotdot; }
  const NumericVector<Number> * solutionUDotOld() const override { return _u_dot_old; }
  const NumericVector<Number> * solutionUDotDotOld() const override { return _u_dotdot_old; }

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution() override;

  // This is an empty function since the Aux system doesn't have a matrix!
  virtual void augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                               std::vector<dof_id_type> & /*n_nz*/,
                               std::vector<dof_id_type> & /*n_oz*/) override;

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
  std::set<std::string> getDependObjects();
  /**
   * Adds a solution length vector to the system.
   *
   * @param vector_name The name of the vector.
   * @param project Whether or not to project this vector when doing mesh refinement.
   *                If the vector is just going to be recomputed then there is no need to project
   * it.
   * @param type What type of parallel vector.  This is usually either PARALLEL or GHOSTED.
   *             GHOSTED is needed if you are going to be accessing off-processor entries.
   *             The ghosting pattern is the same as the solution vector.
   */
  NumericVector<Number> &
  addVector(const std::string & vector_name, const bool project, const ParallelType type) override;

  /**
   * Get the minimum quadrature order for evaluating elemental auxiliary variables
   */
  virtual Order getMinQuadratureOrder() override;

  /**
   * Indicated whether this system needs material properties on boundaries.
   * @return Boolean if IntegratedBCs are active
   */
  bool needMaterialOnSide(BoundaryID bnd_id);

  NumericVector<Number> & solution() override { return *_sys.solution; }
  NumericVector<Number> & solutionOld() override { return *_sys.old_local_solution; }
  NumericVector<Number> & solutionOlder() override { return *_sys.older_local_solution; }
  NumericVector<Number> * solutionPreviousNewton() override { return _solution_previous_nl; }

  const NumericVector<Number> & solution() const override { return *_sys.solution; }
  const NumericVector<Number> & solutionOld() const override { return *_sys.old_local_solution; }
  const NumericVector<Number> & solutionOlder() const override
  {
    return *_sys.older_local_solution;
  }
  const NumericVector<Number> * solutionPreviousNewton() const override
  {
    return _solution_previous_nl;
  }

  virtual TransientExplicitSystem & sys() { return _sys; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  virtual NumericVector<Number> * solutionState(unsigned int i) override;

  virtual void setPreviousNewtonSolution();

  void setScalarVariableCoupleableTags(ExecFlagType type);

  void clearScalarVariableCoupleableTags();

  // protected:
  void computeScalarVars(ExecFlagType type);
  void computeNodalVars(ExecFlagType type);
  void computeNodalVecVars(ExecFlagType type);
  void computeElementalVars(ExecFlagType type);
  void computeElementalVecVars(ExecFlagType type);

  template <typename AuxKernelType>
  void computeElementalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse,
                                  const std::vector<std::vector<MooseVariableFEBase *>> & vars,
                                  const PerfID timer);

  template <typename AuxKernelType>
  void computeNodalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse,
                              const std::vector<std::vector<MooseVariableFEBase *>> & vars,
                              const PerfID timer);

  FEProblemBase & _fe_problem;

  TransientExplicitSystem & _sys;

  /// solution vector from nonlinear solver
  mutable const NumericVector<Number> * _current_solution;
  /// Serialized version of the solution vector
  NumericVector<Number> & _serialized_solution;
  /// Solution vector of the previous nonlinear iterate
  NumericVector<Number> * _solution_previous_nl;
  /// solution vector for u^dot
  NumericVector<Number> * _u_dot;
  /// solution vector for u^dotdot
  NumericVector<Number> * _u_dotdot;

  /// Old solution vector for u^dot
  NumericVector<Number> * _u_dot_old;
  /// Old solution vector for u^dotdot
  NumericVector<Number> * _u_dotdot_old;

  std::vector<NumericVector<Number> *> _solution_state;

  /// Whether or not a copy of the residual needs to be made
  bool _need_serialized_solution;

  // Variables
  std::vector<std::vector<MooseVariableFEBase *>> _nodal_vars;
  std::vector<std::vector<MooseVariableFEBase *>> _nodal_std_vars;
  std::vector<std::vector<MooseVariableFEBase *>> _nodal_vec_vars;

  std::vector<std::vector<MooseVariableFEBase *>> _elem_vars;
  std::vector<std::vector<MooseVariableFEBase *>> _elem_std_vars;
  std::vector<std::vector<MooseVariableFEBase *>> _elem_vec_vars;

  // Storage for AuxScalarKernel objects
  ExecuteMooseObjectWarehouse<AuxScalarKernel> _aux_scalar_storage;

  // Storage for AuxKernel objects
  ExecuteMooseObjectWarehouse<AuxKernel> _nodal_aux_storage;
  ExecuteMooseObjectWarehouse<AuxKernel> _elemental_aux_storage;

  // Storage for VectorAuxKernel objects
  ExecuteMooseObjectWarehouse<VectorAuxKernel> _nodal_vec_aux_storage;
  ExecuteMooseObjectWarehouse<VectorAuxKernel> _elemental_vec_aux_storage;

  /// Timers
  const PerfID _compute_scalar_vars_timer;
  const PerfID _compute_nodal_vars_timer;
  const PerfID _compute_nodal_vec_vars_timer;
  const PerfID _compute_elemental_vars_timer;
  const PerfID _compute_elemental_vec_vars_timer;

  friend class ComputeIndicatorThread;
  friend class ComputeMarkerThread;
  friend class FlagElementsThread;
  friend class ComputeNodalKernelsThread;
  friend class ComputeNodalKernelBcsThread;
  friend class ComputeNodalKernelJacobiansThread;
  friend class ComputeNodalKernelBCJacobiansThread;
};
