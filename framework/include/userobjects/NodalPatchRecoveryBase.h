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
#include "ElementUserObject.h"

class ElementSubdomainModifierBase;

class NodalPatchRecoveryBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryBase(const InputParameters & parameters);

  /**
   * Solve the least-squares problem. Use the fitted coefficients to calculate  the value at the
   * requested point.
   *
   * @param p           Point at which to compute the fitted value
   * @param elem_ids    Ids of the elements in the patch
   * @return The fitted value
   */
  virtual Real nodalPatchRecovery(const Point & p, const std::vector<dof_id_type> & elem_ids) const;

  /**
   * Compute coefficients without reading or writing cached values, which allows this method to be
   * const
   * The coefficients returned by this function are intended to be multiplied with the multi-index
   * table to build the corresponding polynomial. For details on the multi-index table, refer to the
   * comments in the multiIndex() function.
   *
   * @param elem_ids Ids of the elements in the patch
   * @return The coefficients of the polynomial
   */
  const RealEigenVector getCoefficients(const std::vector<dof_id_type> & elem_ids) const;

  /**
   * Compute coefficients, using cached values if available, and store any newly computed
   * coefficients in cache
   * The coefficients returned by this function are intended to be multiplied with the multi-index
   * table to build the corresponding polynomial. For details on the multi-index table, refer to the
   * comments in the multiIndex() function.
   *
   * @param elem_ids Ids of the elements in the patch
   * @return The coefficients of the polynomial
   */
  const RealEigenVector getCachedCoefficients(const std::vector<dof_id_type> & elem_ids);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject &) override;
  void finalize() override;

  /// Returns the multi-index table
  const std::vector<std::vector<unsigned int>> & multiIndex() const { return _multi_index; }

protected:
  /// Compute the quantity to recover using nodal patch recovery
  virtual Real computeValue() = 0;

  unsigned int _qp;

private:
  /// Builds a query map of element IDs that require data from other processors.
  /// This version of this method only checks data communication needs for a provided vector of elements
  /// @param specific_elems Set of elements to consider for data needed from another processor
  /// @return Map of processor id to vector of non-local elements on the current processor that belong to
  ///                that processor. Used to ensure that all non-local evaluable elements are properly synchronized
  std::unordered_map<processor_id_type, std::vector<dof_id_type>>
  gatherSendList(const std::vector<dof_id_type> & specific_elems);

  /// Builds a query map of element IDs that require data from other processors.
  /// This version of this method iterates over all semi-local evaluable elements (including ghost elements)
  /// and records those belonging to a different processor.
  /// @return Map of processor id to vector of semilocal elements on the current processor that belong to
  ///                that processor. Used to ensure that all semi-local evaluable elements are properly synchronized
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> gatherSendList();

  /**
   * Compute the P vector at a given point
   * i.e. given dim = 2, order = 2, polynomial P has the following terms:
   * 1
   * x
   * y
   * x^2
   * xy
   * y^2
   *
   * @param q_point point at which to evaluate the polynomial basis
   */
  RealEigenVector evaluateBasisFunctions(const Point & q_point) const;

  /**
   * @brief Synchronizes local matrices and vectors (_Ae, _be) across processors.
   *
   * @p specific_elems: the elements that will be synchronized.
   * This is typically used when the monomial basis needs to be built from a
   * specific patch, or a subset of that patch.
   */
  void sync(const std::vector<dof_id_type> & specific_elems);

  /**
   * @brief Synchronizes local matrices and vectors (_Ae, _be) across processors.
   * all ghosted evaluable elements are synchronized.
   */
  void sync();

  /**
   * @brief Helper function to perform the actual communication of _Ae and _be.
   */
  void
  syncHelper(const std::unordered_map<processor_id_type, std::vector<dof_id_type>> & query_ids);

  /**
   * @brief Adds an element to the map provided in query_ids if it belongs to a different processor.
   * @param elem Pointer to element to be considered for addition to the map
   * @param query_ids Map of processor id to vector of semilocal elements on the current processor
   *                                   that belong to that processor.
   */
  void addToQuery(const libMesh::Elem * elem,
                  std::unordered_map<processor_id_type, std::vector<dof_id_type>> & query_ids);

  /// The polynomial order, default is variable order
  const unsigned int _patch_polynomial_order;

  /**
   * Generates the multi-index table for a polynomial basis.
   *
   * Each multi-index is a vector of exponents [i, j, k], representing monomials like x^i y^j z^k.
   * The table contains all unique multi-indices where the sum of exponents is <= order.
   *
   * Ordering:
   * - Grouped by total degree (sum of exponents), from 0 to order.
   * - Within each group, ordered colexicographically (last index varies fastest).
   *
   * Examples:
   * dim = 1, order = 3: [0], [1], [2], [3]
   * dim = 2, order = 2: [0,0], [0,1], [1,0], [0,2], [1,1], [2,0]
   * dim = 3, order = 2: [0,0,0], [0,0,1], [0,1,0], [1,0,0], [0,0,2], [0,1,1], [1,0,1], [0,2,0],
   * [1,1,0], [2,0,0]
   *
   * @param dim Number of variables.
   * @param order Maximum total degree.
   * @return Vector of multi-indices.
   */
  const std::vector<std::vector<unsigned int>> _multi_index;

  /// Number of basis functions
  const unsigned int _q;

  /// The element-level A matrix
  std::map<dof_id_type, RealEigenMatrix> _Ae;

  /// The element-level b vector
  std::map<dof_id_type, RealEigenVector> _be;

  /// @brief Cache for least-squares coefficients used in nodal patch recovery.
  /// Typically, there is a one-to-one mapping from element to coefficients,
  /// so only a single set of coefficients is cached rather than a full map.
  mutable std::vector<dof_id_type> _cached_elem_ids;
  mutable RealEigenVector _cached_coef;

  /// @brief Whether the mesh is distributed
  bool _distributed_mesh;

  /// @brief The processor IDs vector in the running
  std::vector<int> _proc_ids;

  friend class ElementSubdomainModifierBase;
};
