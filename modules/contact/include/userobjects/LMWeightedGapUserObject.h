//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedGapUserObject.h"
#include "RankFourTensor.h"
#include "MaterialProperty.h"

#include <array>

#include "libmesh/fe_base.h"
#include "libmesh/quadrature_gauss.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename>
class MooseVariableFE;

/**
 * User object for computing weighted gaps and contact pressure for Lagrange multipler based
 * mortar constraints
 */
class LMWeightedGapUserObject : virtual public WeightedGapUserObject
{
public:
  static InputParameters validParams();
  /**
   * New parameters that this sub-class introduces
   */
  static InputParameters newParams();

  LMWeightedGapUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactPressure() const override;
  virtual void reinit() override;
  virtual Real getNormalContactPressure(const Node * const /*node*/) const override;

  virtual void initialize() override;
  virtual void finalize() override;

  virtual Real nodalScale(const DofObject * const dof) const override
  {
    return findValue(_dof_to_nodal_scale, dof, Real(1));
  }
  virtual bool usesNodalScaling() const override { return _use_nodal_scaling; }

  virtual Real normalizeCDivisor(const DofObject * const dof,
                                 const Real covered_normalization) const override
  {
    return _use_nodal_scaling ? findValue(_dof_to_full_normalization, dof, covered_normalization)
                              : covered_normalization;
  }

  virtual void timestepSetup() override;
  virtual void meshChanged() override;

  /**
   * Per-node physical normal stiffness scale and its accumulated mortar weight.
   * Only populated when derive_c_from_elasticity = true; after finalize() the scale has already
   * been divided by the accumulated weight.
   */
  const std::unordered_map<const DofObject *, std::array<Real, 2>> & dofToDerivedC() const;

  /// Whether 'derive_c_from_elasticity = true' (c_normal_strategy = physical), i.e. whether
  /// contactPressure()/getNormalContactPressure() apply the dofToDerivedC() scale to raw LM dof
  /// values rather than returning them unscaled.
  bool deriveCFromElasticity() const { return _derive_c_from_elasticity; }

protected:
  virtual void computeQpIProperties() override;
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return true; }

  /// The node-based scaling steps, kept out of initialize()/finalize()/computeQpIProperties() so
  /// that a class inheriting this one through more than one path can run them without invoking the
  /// base class twice.
  void initializeNodalScaling();
  void finalizeNodalScaling();
  void computeQpINodalScaling();

  /**
   * Full coordinate-weighted integral int_e N_j per local node on the secondary lower-dimensional
   * element (exact in RZ and on warped elements), cached per element: the denominator of kappa_j
   * (Popp 2013 eq. 36).
   * @param elem The secondary lower-dimensional element
   * @return Per-local-node full shape-function integrals
   */
  const std::vector<Real> & fullNodalIntegrals(const Elem * elem);

  /**
   * Check user input validity for provided variable
   */
  void checkInput(const MooseVariable * const var, const std::string & var_name) const;

  /**
   * Verify that the provided variables have degrees of freedom at nodes
   */
  void verifyLagrange(const MooseVariable & var, const std::string & var_name) const;

  // Non-virtual helpers so diamond-derived classes can call them without re-entering the base chain
  void clearDerivedC();
  void finalizeDerivedC();
  void accumulateDerivedCIfNeeded();

  /**
   * Interpolate a lower-dimensional Lagrange multiplier variable's nodal values onto the current
   * quadrature points, scaling each node's contribution by its derived physical stiffness
   * (dofToDerivedC()) before interpolating. This implements the x = D*y change of variables for a
   * physical LM variable whose raw (persistently stored) dof value is the non-physical y; the
   * per-node scale D can vary across a mortar segment element, so this cannot be expressed as a
   * single scalar multiplying the unscaled interpolated field.
   */
  const ADVariableValue & scaledLowerSln(const MooseVariableFE<Real> & lm_var,
                                         ADVariableValue & cache) const;

  /// The derived physical stiffness scale D_j relating the stored LM value y_j to the multiplier
  /// D_j y_j; 1 unless derive_c_from_elasticity = true
  Real derivedPressureScale(const DofObject * dof) const
  {
    return _derive_c_from_elasticity ? libmesh_map_find(_dof_to_derived_c, dof)[0] : 1;
  }

  /// Whether to derive the physical normal stiffness from elasticity tensor material properties
  const bool _derive_c_from_elasticity;

  /// Whether the elasticity tensor material property was declared as an AD property
  const bool _use_automatic_differentiation;

  /// Per-node accumulated (physical stiffness scale * weight, weight)
  std::unordered_map<const DofObject *, std::array<Real, 2>> _dof_to_derived_c;

  /// Whether the physical stiffness scale must be refreshed at the next mortar execution
  bool _derived_c_needs_update = true;

  /// The Lagrange multiplier variable representing the contact pressure
  const MooseVariableFE<Real> * const _lm_var;

  /// Whether to use Petrov-Galerkin approach
  const bool _use_petrov_galerkin;

  /// The auxiliary Lagrange multiplier variable (used together whith the Petrov-Galerkin approach)
  const MooseVariable * const _aux_lm_var;

  /// Physical contact pressure sum_j Phi_j (D_j y_j / kappa_j) at the segment quadrature points when
  /// node-based or derived physical scaling is active; recomputed once per segment in reinit() (see
  /// contactPressure()).
  ADVariableValue _scaled_contact_pressure;

  /// Whether to apply the Popp et al. (2013) node-based Lagrange-multiplier scaling (kappa_j) for
  /// improved conditioning of partially covered (edge-dropping) secondary elements
  const bool _use_nodal_scaling;

  /// Per-node numerator of kappa_j (Popp 2013 eq. 36), summed over adjacent secondary elements;
  /// finalizeNodalScaling() divides by the adjacency count
  std::unordered_map<const DofObject *, Real> _dof_to_covered_fraction_sum;

  /// A map from node to its node-based scaling factor kappa_j (see nodalScale())
  std::unordered_map<const DofObject *, Real> _dof_to_nodal_scale;

  /// Cache of the per-node full integrals int_e N_j (see fullNodalIntegrals()), keyed by element id;
  /// cleared each evaluation since the displaced geometry changes
  std::unordered_map<dof_id_type, std::vector<Real>> _elem_to_full_nodal_integral;

  /// Per-node sum, over each distinct adjacent secondary element, of the full-element integral
  /// int_e N_j (see fullNodalIntegrals()); the coverage-independent divisor for `normalize_c` when
  /// node-based scaling is active (see normalizeCDivisor()), reduces exactly to the covered-region
  /// `normalization` (WeightedGapUserObject::computeQpIProperties()) at full coverage, since
  /// int_e Phi_j = int_e N_j for the dual basis by construction. Not from Popp (2013); a PR-review
  /// proposal for composing `normalize_c` with node-based scaling.
  std::unordered_map<const DofObject *, Real> _dof_to_full_normalization;

  /// Elements already folded into _dof_to_full_normalization for a given node, to avoid double
  /// counting when multiple mortar segments cover the same element
  std::unordered_map<const DofObject *, std::unordered_set<dof_id_type>> _full_normalization_elems;

  /// Finite element and quadrature rule used to evaluate fullNodalIntegrals()
  std::unique_ptr<libMesh::FEBase> _nodal_scaling_fe;
  std::unique_ptr<libMesh::QGauss> _nodal_scaling_qrule;

private:
  template <bool is_ad>
  void fetchElasticityTensorProperties(const std::string & sec_name, const std::string & pri_name);

  template <bool is_ad>
  void accumulateDerivedC();

  /// Non-AD elasticity tensor on secondary side (non-null when !_use_automatic_differentiation)
  const GenericMaterialProperty<RankFourTensor, false> * _elasticity_tensor_secondary = nullptr;
  /// Non-AD elasticity tensor on primary side
  const GenericMaterialProperty<RankFourTensor, false> * _elasticity_tensor_primary = nullptr;
  /// AD elasticity tensor on secondary side (non-null when _use_automatic_differentiation)
  const GenericMaterialProperty<RankFourTensor, true> * _elasticity_tensor_secondary_ad = nullptr;
  /// AD elasticity tensor on primary side
  const GenericMaterialProperty<RankFourTensor, true> * _elasticity_tensor_primary_ad = nullptr;
};

inline const std::unordered_map<const DofObject *, std::array<Real, 2>> &
LMWeightedGapUserObject::dofToDerivedC() const
{
  return _dof_to_derived_c;
}
