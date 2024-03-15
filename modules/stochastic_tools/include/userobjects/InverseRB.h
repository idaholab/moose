///* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DEIMRBMapping.h"
#include "GeneralUserObject.h"
#include "MappingInterface.h"
#include "NonlinearSystemBase.h"
#include "SystemBase.h"
#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/id_types.h"
#include <memory>

/**
 * InverseRB is a user object that performs inverse reduced basis mapping.
 */
class InverseRB : public GeneralUserObject, public MappingInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor for InverseRB.
   * @param parameters Input parameters
   */
  InverseRB(const InputParameters & parameters);

  virtual void initialize() override;

  virtual void execute() override;

  virtual void finalize() override;

  virtual void initialSetup() override;

  // only needed for ElementUserObjects and NodalUseroObjects
  virtual void threadJoin(const UserObject & /*y*/) override{};

protected:
  /// Link to the mapping object which provides the inverse mapping function
  DEIMRBMapping * _mapping;

  std::vector<dof_id_type> _residual_inds;

  std::vector<std::pair<dof_id_type, dof_id_type>> _jacobian_matrix_inds;

  const DofMap & _dof_map;

  const dof_id_type _max_iter;

  const Real _tolerance;

  const Real _relax_factor;

private:
  /**
   * Finds the reduced element range for the given DOFs.
   * @param dofs Vector of DOF indices
   * @return Vector of pointers to the reduced elements
   */
  std::vector<const Elem *> findReducedElemRange(const std::vector<dof_id_type> & dofs);

  /**
   * Finds the reduced node range for the given DOFs.
   * @param dofs Vector of DOF indices
   * @return Vector of pointers to the reduced nodes
   */
  std::vector<const Node *> findReducedNodeRange(const std::vector<dof_id_type> & dofs);

  /**
   * Creates a range object from a vector of items.
   * @tparam RangeType Type of the range object
   * @tparam ItemType Type of the items in the vector
   * @tparam IteratorType Type of the iterator for the range object
   * @param items Vector of pointers to the items
   * @return Unique pointer to the created range object
   */
  template <typename RangeType, typename ItemType, typename IteratorType>
  std::unique_ptr<RangeType> createRangeFromVector(const std::vector<const ItemType *> & items);

  /**
   * Retrieves the reduced Jacobian values from the sparse matrix.
   * @param jac Sparse Jacobian matrix
   * @return Vector of reduced Jacobian values
   */
  std::vector<Real> getReducedJacValues(const SparseMatrix<Number> & jac);

  /**
   * Retrieves the reduced residual values from the numeric vector.
   * @param res Numeric residual vector
   * @return Reduced residual values
   */
  std::vector<Real> getReducedResValues(const NumericVector<Number> & res);

  /**
   * Computes the reduced Jacobian matrix.
   * @return Reduced Jacobian
   */
  DenseMatrix<Real> computeReducedJacobian();

  /**
   * Computes the reduced residual vector.
   * @return Reduced residual
   */
  DenseVector<Real> computeReducedResidual();

  /**
   * Updates the solution vector with the reduced solution.
   * @param reduced_sol Reduced solution vector
   */
  void updateSolution(const DenseVector<Real> & reduced_sol);

  /**
   * Computes the residual norm.
   * @return Residual norm value
   */
  Real computeResidual();

  /// Vector of reduced Jacobian elements
  std::vector<const Elem *> _red_jac_elem;

  /// Vector of reduced residual elements
  std::vector<const Elem *> _red_res_elem;

  /// Vector of reduced Jacobian nodes
  std::vector<const Node *> _red_jac_node;

  /// Vector of reduced residual nodes
  std::vector<const Node *> _red_res_node;

  /// Local range of reduced Jacobian elements
  std::unique_ptr<ConstElemRange> _red_jac_elem_local_range;

  /// Local range of reduced residual elements
  std::unique_ptr<ConstElemRange> _red_res_elem_local_range;

  /// Local range of reduced Jacobian nodes
  std::unique_ptr<ConstNodeRange> _red_jac_node_local_range;

  /// Local range of reduced residual nodes
  std::unique_ptr<ConstNodeRange> _red_res_node_local_range;

  /// Nonlinear system
  NonlinearSystemBase & _nl_sys;

  /// Jacobian matrix
  SparseMatrix<Number> * _jac_matrix;

  /// Residual vector
  NumericVector<Number> * _residual;

  /// Current solution
  const NumericVector<Number> * _curr_sol;

  /// Linear solver
  LinearSolver<Number> * _solver;
};
