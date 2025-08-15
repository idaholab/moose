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
   * Const version of getCoefficients. This will always recompute the coefficients.
   *
   * @param elem_ids Ids of the elements in the patch
   * @return The coefficients of the polynomial
   */
  const RealEigenVector getCoefficientsNoCache(const std::vector<dof_id_type> & elem_ids) const;

  /**
   * Non-const version of getCoefficients. This will recompute the coefficients
   * even if they are already cached.
   *
   * @param elem_ids Ids of the elements in the patch
   * @return The coefficients of the polynomial
   */
  const RealEigenVector getCoefficients(const std::vector<dof_id_type> & elem_ids);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject &) override;
  void finalize() override;

  /**
   * @brief Synchronizes local matrices and vectors (_Ae, _be) across processors.
   *
   * If @p specific_elems is provided, only those elements will be synchronized.
   * This is typically used when the monomial basis needs to be built from a
   * specific patch, or a subset of that patch.
   *
   * If @p specific_elems is not provided, all ghosted evaluable elements are
   * synchronized (default behavior).
   */
  void sync(const std::optional<std::vector<dof_id_type>> & specific_elems = std::nullopt);

  /// Returns the multi-index table
  const std::vector<std::vector<unsigned int>> & multiIndex() const { return _multi_index; }

protected:
  /// Compute the quantity to recover using nodal patch recovery
  virtual Real computeValue() = 0;

  unsigned int _qp;

private:
  /// Builds a query map of element IDs that need data from other processors.
  /// There are two modes of operation:
  /// (1) If @p specific_elems is not provided: iterate over all semi-local
  ///     evaluable elements (including ghost elements) and record those that
  ///     belong to a different processor.
  /// (2) If @p specific_elems is provided: only iterate over the given elements,
  ///     and record those that belong to a different processor.
  ///
  /// This ensures that "semi-local evaluable elements" are synchronized in the default
  /// case, while still allowing callers to explicitly request synchronization
  /// for a targeted subset when needed.
  std::unordered_map<processor_id_type, std::vector<dof_id_type>>
  gatherSendList(const std::optional<std::vector<dof_id_type>> & specific_elems = std::nullopt);

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

  /// The polynomial order, default is variable order
  const unsigned int _patch_polynomial_order;

  /// The multi-index table
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

  std::vector<dof_id_type> removeDuplicates(const std::vector<dof_id_type> & ids) const;
};
