//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#ifdef NEML2_ENABLED

// system includes
#include <unordered_map>

// libmesh includes
#include "MooseTypes.h"
#include "libmesh/fe_type.h"
#include "libmesh/id_types.h"

// MOOSE includes
#include "ElementUserObject.h"
#include "NEML2Assembly.h"
#include "MooseVariableFE.h"

// NEML2 includes
#include "neml2/tensors/Tensor.h"

/**
 * @brief This user object serves as the "interface" for interpolating MOOSE variable values and
 * gradients from the current solution vector.
 *
 * Other objects can "couple" to variable values and/or gradients using through this interface.
 */
class NEML2FEInterpolation : public ElementUserObject
{
public:
  static InputParameters validParams();

  NEML2FEInterpolation(const InputParameters & parameters);

  /**
   * @brief Get the variable value of a MOOSE nonlinear variable converted to a NEML2 tensor.
   *
   * The NEML2 tensor will have shape (nelem, nqp;), where nelem is the number of elements, and nqp
   * is the number of quadrature points per element. The tensor is stored in the device memory.
   *
   * @warning The tensor value is only available after finalize() is called.
   */
  const neml2::Tensor & getValue(const std::string & var_name);

  /**
   * @brief Get the variable gradient of a MOOSE nonlinear variable converted to a NEML2 tensor.
   *
   * The NEML2 tensor will have shape (nelem, nqp; 3), where nelem is the number of elements,
   * and nqp is the number of quadrature points per element. The tensor is stored in the device
   * memory.
   *
   * @warning The tensor value is only available after finalize() is called.
   */
  const neml2::Tensor & getGradient(const std::string & var_name);

  /**
   * @brief Get the shape function associated with a MOOSE variable.
   *
   * The NEML2 tensor will have shape (nelem, ndofe, nqp;), where nelem is the number of elements,
   * nqp is the number of quadrature points per element, and ndofe is the number of degrees of
   * freedom per element. The tensor is stored in the device memory.
   *
   * @warning The tensor value is only available after finalize() is called.
   */
  const neml2::Tensor & getPhi(const std::string & var_name);

  /**
   * @brief Get the shape function gradient associated with a MOOSE variable.
   *
   * The NEML2 tensor will have shape (nelem, ndofe, nqp; 3), where nelem is the number of elements,
   * nqp is the number of quadrature points per element, and ndofe is the number of degrees of
   * freedom per element. The tensor is stored in the device memory.
   *
   * @warning The tensor value is only available after finalize() is called.
   */
  const neml2::Tensor & getPhiGradient(const std::string & var_name);

  /**
   * @brief Get the local dof map associated with a MOOSE variable.
   *
   * The NEML2 tensor will have shape (nelem, ndofe;), where nelem is the number of elements,
   * and ndofe is the number of degrees of freedom per element. The tensor is stored in the device
   * memory.
   *
   * @warning The tensor value is only available after finalize() is called.
   */
  const neml2::Tensor & getDofMap(const std::string & var_name);

  /// Similar to getDofMap, but returns the global dof map (as a flattened vector of dof_id_type)
  const std::vector<dof_id_type> & getGlobalDofMap(const std::string & var_name);

  int64_t local_ndof() const;

  /// Whether the current FEM context is up to date
  bool contextUpToDate() const { return _fem_context_up_to_date; }

  /// Invalidate the cached FEM context such as dof map, shape functions, etc.
  void invalidateFEMContext();

  /// Invalidate the cached interpolations
  void invalidateInterpolations();

  void initialSetup() override;
  void meshChanged() override;

  void initialize() override;
  void execute() override;
  void finalize() override;
  void threadJoin(const UserObject &) override;

protected:
  virtual void syncWithMainThread();
  virtual void updateFEMContext();
  virtual void updateDofMap();
  virtual void updatePhi();
  virtual void updateGradPhi();
  virtual void updateInterpolations();

  /// Assembly
  const NEML2Assembly & _neml2_assembly;

  /// Whether the current FEM context is up to date
  bool _fem_context_up_to_date = false;

  /// Whether the current interpolations are up to date
  bool _interp_up_to_date = false;

  /// coupled variables (by value) requested by other objects
  std::unordered_map<std::string, neml2::Tensor> _vars;

  /// coupled variables (by gradient) requested by other objects
  std::unordered_map<std::string, neml2::Tensor> _grad_vars;

  /// moose variables that have been coupled
  std::unordered_map<std::string, const MooseVariableFE<Real> *> _moose_vars;

  /// cached information on the requested function spaces
  ///@{
  std::unordered_map<FEType, int64_t> _ndofe;
  std::unordered_map<FEType, const VariablePhiValue *> _phis;
  std::unordered_map<FEType, const VariablePhiGradient *> _grad_phis;

  std::unordered_map<std::string, std::vector<int64_t>> _moose_dof_map;
  std::unordered_map<std::string, std::vector<dof_id_type>> _moose_dof_map_global;
  std::unordered_map<FEType, std::vector<Real>> _moose_phi;
  std::unordered_map<FEType, std::vector<Real>> _moose_grad_phi;

  std::unordered_map<std::string, neml2::Tensor> _neml2_dof_map;
  std::unordered_map<FEType, neml2::Tensor> _neml2_phi;
  std::unordered_map<FEType, neml2::Tensor> _neml2_grad_phi;
  ///@}

  /// PETSc solution vector
  const PetscVector<Real> * _petsc_solution;

private:
  /// Helper to get the MOOSE variable and check for common restrictions
  const MooseVariableFE<Real> * getMOOSEVariable(const std::string & var_name) const;

  /// Helper vector to store local dof indices
  std::vector<dof_id_type> _dof_indices;

  /// Number of local dofs (including ghost dofs)
  int64_t _local_ndof;
};

#endif
