//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

#include <unordered_map>
#include <unordered_set>

#include "libmesh/system.h"
#include "libmesh/transient_system.h"

// Forward declarations
class AuxKernelBase;
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
   * Adds an auxiliary kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Parameters for this kernel
   */
  void addKernel(const std::string & kernel_name,
                 const std::string & name,
                 InputParameters & parameters);

#ifdef MOOSE_KOKKOS_ENABLED
  void addKokkosKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters & parameters);
#endif

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
  virtual void reinitElemFace(const Elem * elem, unsigned int side, THREAD_ID tid) override;

  const NumericVector<Number> * const & currentSolution() const override
  {
    return _current_solution;
  }

  virtual void serializeSolution();

  // This is an empty function since the Aux system doesn't have a matrix!
  virtual void augmentSparsity(libMesh::SparsityPattern::Graph & /*sparsity*/,
                               std::vector<dof_id_type> & /*n_nz*/,
                               std::vector<dof_id_type> & /*n_oz*/) override;

  /**
   * Compute auxiliary variables
   * @param type Time flag of which variables should be computed
   */
  virtual void compute(ExecFlagType type) override;

#ifdef MOOSE_KOKKOS_ENABLED
  void kokkosCompute(ExecFlagType type);
#endif

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
  virtual libMesh::Order getMinQuadratureOrder() override;

  /**
   * Indicated whether this system needs material properties on boundaries.
   * @return Boolean if IntegratedBCs are active
   */
  bool needMaterialOnSide(BoundaryID bnd_id);

  virtual libMesh::System & sys() { return _sys; }

  virtual libMesh::System & system() override { return _sys; }
  virtual const libMesh::System & system() const override { return _sys; }

  /// Access the stored raw cell-centered gradient components for linear FV auxiliary variables.
  const std::vector<std::unique_ptr<NumericVector<Number>>> & linearFVGradientContainer() const;

  /// Request storage and assembly of limiter-specific cell gradients.
  void requestLinearFVLimitedGradients(const Moose::FV::GradientLimiterType limiter_type);

  /// Access the stored raw or limited cell-centered gradient components.
  const std::vector<std::unique_ptr<NumericVector<Number>>> &
  linearFVLimitedGradientContainer(const Moose::FV::GradientLimiterType limiter_type) const;

  /// Access the limiter types requested for this auxiliary system.
  const std::unordered_set<Moose::FV::GradientLimiterType> &
  requestedLinearFVLimitedGradientTypes() const;

  /// Copies the current solution into the previous nonlinear iteration solution
  virtual void copyCurrentIntoPreviousNL();

  void setScalarVariableCoupleableTags(ExecFlagType type);

  void clearScalarVariableCoupleableTags();

  const ExecuteMooseObjectWarehouse<AuxKernel> & nodalAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<VectorAuxKernel> & nodalVectorAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<ArrayAuxKernel> & nodalArrayAuxWarehouse() const;

  const ExecuteMooseObjectWarehouse<AuxKernel> & elemAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<VectorAuxKernel> & elemVectorAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<ArrayAuxKernel> & elemArrayAuxWarehouse() const;

#ifdef MOOSE_KOKKOS_ENABLED
  const ExecuteMooseObjectWarehouse<AuxKernelBase> & kokkosNodalAuxWarehouse() const;
  const ExecuteMooseObjectWarehouse<AuxKernelBase> & kokkosElemAuxWarehouse() const;
#endif

  /// Computes and stores ||current - old|| / ||current|| for each variable in the given vector
  /// @param var_diffs a vector being filled with the L2 norm of the solution difference
  void variableWiseRelativeSolutionDifferenceNorm(std::vector<Number> & var_diffs) const;

protected:
  void computeScalarVars(ExecFlagType type);
  void computeNodalVars(ExecFlagType type);
  void computeMortarNodalVars(ExecFlagType type);
  void computeNodalVecVars(ExecFlagType type);
  void computeNodalArrayVars(ExecFlagType type);
  void computeElementalVars(ExecFlagType type);
  void computeElementalVecVars(ExecFlagType type);
  void computeElementalArrayVars(ExecFlagType type);
  /// Compute and store the Green-Gauss gradients for linear FV auxiliary variables.
  void computeGradients();

  template <typename AuxKernelType>
  void computeElementalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse);

  template <typename AuxKernelType>
  void computeNodalVarsHelper(const MooseObjectWarehouse<AuxKernelType> & warehouse);

  /**
   * Return temporary storage for gradients during gradient assembly.
   * The returned vectors are scratch storage and are moved into the final gradient
   * container at the end of the assembly.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> & temporaryLinearFVGradientContainer()
  {
    return _temporary_gradient;
  }

  /**
   * Return temporary storage for limited gradients during gradient assembly.
   * The returned vectors are scratch storage and are moved into the final limited-gradient
   * container at the end of the assembly.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> &
  temporaryLinearFVLimitedGradientContainer(const Moose::FV::GradientLimiterType limiter_type)
  {
    return _temporary_limited_gradient[limiter_type];
  }

  libMesh::System & _sys;

  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;

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

  /// Scratch storage for raw gradients assembled during the current compute pass.
  std::vector<std::unique_ptr<NumericVector<Number>>> _temporary_gradient;

  /// Persisted raw cell-centered gradient components keyed by spatial direction.
  std::vector<std::unique_ptr<NumericVector<Number>>> _raw_grad_container;

  /// Set of requested limiter types for which limited gradients should be computed.
  std::unordered_set<Moose::FV::GradientLimiterType> _requested_limited_gradient_types;

  /// Persisted limited gradient components keyed by limiter type.
  std::unordered_map<Moose::FV::GradientLimiterType,
                     std::vector<std::unique_ptr<NumericVector<Number>>>>
      _raw_limited_grad_containers;

  /// Scratch storage for limited gradients assembled during the current compute pass.
  std::unordered_map<Moose::FV::GradientLimiterType,
                     std::vector<std::unique_ptr<NumericVector<Number>>>>
      _temporary_limited_gradient;

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

#ifdef MOOSE_KOKKOS_ENABLED
  // Storage for KokkosAuxKernel objects
  ExecuteMooseObjectWarehouse<AuxKernelBase> _kokkos_nodal_aux_storage;
  ExecuteMooseObjectWarehouse<AuxKernelBase> _kokkos_elemental_aux_storage;
#endif

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

#ifdef MOOSE_KOKKOS_ENABLED
inline const ExecuteMooseObjectWarehouse<AuxKernelBase> &
AuxiliarySystem::kokkosNodalAuxWarehouse() const
{
  return _kokkos_nodal_aux_storage;
}

inline const ExecuteMooseObjectWarehouse<AuxKernelBase> &
AuxiliarySystem::kokkosElemAuxWarehouse() const
{
  return _kokkos_elemental_aux_storage;
}
#endif
