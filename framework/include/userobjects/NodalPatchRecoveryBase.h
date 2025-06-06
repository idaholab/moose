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

// Hash and equality function for using std::vector<dof_id_type> as unordered_map key
inline void
hash_combine(std::size_t & seed, std::size_t value)
{
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct DofIDVectorHash
{
  std::size_t operator()(const std::vector<dof_id_type> & vec) const
  {
    std::size_t seed = vec.size(); // Include size to differentiate [1,2] and [1,2,0]
    for (const auto & id : vec)
      hash_combine(seed, std::hash<dof_id_type>()(id));
    return seed;
  }
};

struct DofIDVectorEqual
{
  bool operator()(const std::vector<dof_id_type> & a, const std::vector<dof_id_type> & b) const
  {
    return a == b;
  }
};

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

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override;
  virtual void finalize() override;

  void cacheAdditionalElements(const std::vector<dof_id_type> & additional_elems) const
  {
    _additional_elems = additional_elems;
  }

  void cleanQueryIDsAndAdditionalElements() const
  {
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
  mutable std::
      unordered_map<std::vector<dof_id_type>, RealEigenVector, DofIDVectorHash, DofIDVectorEqual>
          _cached_coef;

  /// Print coefficients of the polynomial to console
  const bool _verbose;

  VariableName _var_name; ///< Variable name
};
