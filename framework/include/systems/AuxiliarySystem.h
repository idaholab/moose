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
typedef AuxKernelTempl<RealEigenVector> ArrayAuxKernel;
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

  virtual void initialSetup() override;
  virtual void timestepSetup() override;
  virtual void customSetup(const ExecFlagType & exec_type) override;
  virtual void subdomainSetup() override;
  virtual void residualSetup() override;
  virtual void jacobianSetup() override;
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
   * Get the minimum quadrature order for evaluating elemental auxiliary variables
   */
  virtual Order getMinQuadratureOrder() override;

  /**
   * Indicated whether this system needs material properties on boundaries.
   * @return Boolean if IntegratedBCs are active
   */
  bool needMaterialOnSide(BoundaryID bnd_id);

  virtual ExplicitSystem & sys() { return _sys; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  virtual void setPreviousNewtonSolution();

  void setScalarVariableCoupleableTags(ExecFlagType type);

  void clearScalarVariableCoupleableTags();

  const ExecuteMooseObjectWarehouse<AuxKernel> & nodalAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<VectorAuxKernel> & nodalVectorAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<ArrayAuxKernel> & nodalArrayAuxWarehouse() const;

  const ExecuteMooseObjectWarehouse<AuxKernel> & elemAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<VectorAuxKernel> & elemVectorAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<ArrayAuxKernel> & elemArrayAuxWarehouse() const;

protected:
  void computeScalarVars(ExecFlagType type);
  void computeNodalVars(ExecFlagType type);
  void computeMortarNodalVars(ExecFlagType type);
  void computeNodalVecVars(ExecFlagType type);
  void computeNodalArrayVars(ExecFlagType type);
  void computeElementalVars(ExecFlagType type);
  void computeElementalVecVars(ExecFlagType type);
  void computeElementalArrayVars(ExecFlagType type);

  template <typename AuxKernelType>
  void computeElementalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse);

  template <typename AuxKernelType>
  void computeNodalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse);

  FEProblemBase & _fe_problem;

  ExplicitSystem & _sys;

  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;
  /// Serialized version of the solution vector, or nullptr if a
  /// serialized solution is not needed
  std::unique_ptr<NumericVector<Number>> _serialized_solution;
  /// solution vector for u^dot
  NumericVector<Number> * _u_dot;
  /// solution vector for u^dotdot
  NumericVector<Number> * _u_dotdot;

  /// Old solution vector for u^dot
  NumericVector<Number> * _u_dot_old;
  /// Old solution vector for u^dotdot
  NumericVector<Number> * _u_dotdot_old;

  /// The current states of the solution (0 = current, 1 = old, etc)
  std::vector<NumericVector<Number> *> _solution_state;

  // Variables
  std::vector<std::vector<MooseVariableFEBase *>> _nodal_vars;

  ///@{
  /**
   * Elemental variables. These may be either finite element or finite volume variables
   */
  std::vector<std::vector<MooseVariableFieldBase *>> _elem_vars;
  ///@}

  // Storage for AuxScalarKernel objects
  ExecuteMooseObjectWarehouse<AuxScalarKernel> _aux_scalar_storage;

  // Storage for AuxKernel objects
  ExecuteMooseObjectWarehouse<AuxKernel> _nodal_aux_storage;
  ExecuteMooseObjectWarehouse<AuxKernel> _mortar_nodal_aux_storage;
  ExecuteMooseObjectWarehouse<AuxKernel> _elemental_aux_storage;

  // Storage for VectorAuxKernel objects
  ExecuteMooseObjectWarehouse<VectorAuxKernel> _nodal_vec_aux_storage;
  ExecuteMooseObjectWarehouse<VectorAuxKernel> _elemental_vec_aux_storage;

  // Storage for ArrayAuxKernel objects
  ExecuteMooseObjectWarehouse<ArrayAuxKernel> _nodal_array_aux_storage;
  ExecuteMooseObjectWarehouse<ArrayAuxKernel> _elemental_array_aux_storage;

  friend class ComputeIndicatorThread;
  friend class ComputeMarkerThread;
  friend class FlagElementsThread;
  friend class ComputeNodalKernelsThread;
  friend class ComputeNodalKernelBcsThread;
  friend class ComputeNodalKernelJacobiansThread;
  friend class ComputeNodalKernelBCJacobiansThread;

  NumericVector<Number> & solutionInternal() const override { return *_sys.solution; }
};

inline const ExecuteMooseObjectWarehouse<AuxKernel> &
AuxiliarySystem::nodalAuxWarehouse() const
{
  return _nodal_aux_storage;
}

inline const ExecuteMooseObjectWarehouse<VectorAuxKernel> &
AuxiliarySystem::nodalVectorAuxWarehouse() const
{
  return _nodal_vec_aux_storage;
}

inline const ExecuteMooseObjectWarehouse<ArrayAuxKernel> &
AuxiliarySystem::nodalArrayAuxWarehouse() const
{
  return _nodal_array_aux_storage;
}

inline const ExecuteMooseObjectWarehouse<AuxKernel> &
AuxiliarySystem::elemAuxWarehouse() const
{
  return _elemental_aux_storage;
}

inline const ExecuteMooseObjectWarehouse<VectorAuxKernel> &
AuxiliarySystem::elemVectorAuxWarehouse() const
{
  return _elemental_vec_aux_storage;
}

inline const ExecuteMooseObjectWarehouse<ArrayAuxKernel> &
AuxiliarySystem::elemArrayAuxWarehouse() const
{
  return _elemental_array_aux_storage;
}
