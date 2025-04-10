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
   * Get the coefficients of the polynomial for the given element IDs.
   * If the coefficients are already cached, return them directly.
   *
   * @param elem_ids    Ids of the elements in the patch
   * @return The coefficients of the polynomial
   */
  const RealEigenVector getCoefficients(const std::vector<dof_id_type> & elem_ids) const;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override;
  virtual void finalize() override;

  /**
   * Cache additional elements for the patch recovery.
   *
   * @param additional_elems    Ids of the additional elements to cache
   * @param do_synchronize      Whether to synchronize the A and b vectors across processors
   */
  void cacheAdditionalElements(const std::vector<dof_id_type> & additional_elems,
                               bool do_synchronize = false) const;

  void cleanQueryIDsAndAdditionalElements() const
  {
    if (!_use_specific_elements)
      return;

    _query_ids.clear();
    _additional_elems.clear();
  }

  void identifyAdditionalElementsFromOtherProcs() const;

  /// @brief Synchronizes local matrices and vectors (_Ae, _be) across processors
  /// by gathering and distributing data for specified element IDs (_query_ids)
  /// in a parallel computing environment.
  void synchronizeAebe() const;

  /// Returns the variable name
  virtual const VariableName & variableName() const { return _var_name; }

  /// Returns the multi-index table
  const std::vector<std::vector<unsigned int>> & multiIndex() const { return _multi_index; }

protected:
  /// Compute the quantity to recover using nodal patch recovery
  virtual Real computeValue() = 0;
  void setVariableName(const VariableName & var_name) { _var_name = var_name; }

  unsigned int _qp;

private:
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
  mutable std::map<dof_id_type, RealEigenMatrix> _Ae;

  /// The element-level b vector
  mutable std::map<dof_id_type, RealEigenVector> _be;

  // Map to track which elements are needed from each processor
  mutable std::unordered_map<processor_id_type, std::vector<dof_id_type>> _query_ids;

  /// Additional elements to query
  mutable std::vector<dof_id_type> _additional_elems;

  /// Iterates over all evaluable elements and records their IDs in a query map
  /// if they belong to a different processor.
  void identifyGhostElementsFromOtherProcs() const;

  /// @brief Whether we want to specify the elements for the patch recovery
  bool _use_specific_elements;

  /// @brief Cache for least-squares coefficients used in nodal patch recovery.
  /// Typically, there is a one-to-one mapping from element to coefficients,
  /// so only a single set of coefficients is cached rather than a full map.
  mutable std::vector<dof_id_type> _cached_elem_ids;
  mutable RealEigenVector _cached_coef;

  /// Print coefficients of the polynomial to console
  const bool _verbose;

  /// Variable name
  VariableName _var_name;
};
