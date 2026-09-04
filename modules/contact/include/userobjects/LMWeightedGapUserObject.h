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

#include "libmesh/fe_base.h"
#include "libmesh/quadrature_gauss.h"

#include <memory>
#include <unordered_map>
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

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return true; }
  virtual void computeQpIProperties() override;

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

  /// The Lagrange multiplier variable representing the contact pressure
  const MooseVariableFE<Real> * const _lm_var;

  /// Whether to use Petrov-Galerkin approach
  const bool _use_petrov_galerkin;

  /// The auxiliary Lagrange multiplier variable (used together whith the Petrov-Galerkin approach)
  const MooseVariable * const _aux_lm_var;

  /// Physical contact pressure sum_j Phi_j (zhat_j / kappa_j) at the segment quadrature points when
  /// node-based scaling is active; recomputed once per segment in reinit() (see contactPressure()).
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

  /// Finite element and quadrature rule used to evaluate fullNodalIntegrals()
  std::unique_ptr<libMesh::FEBase> _nodal_scaling_fe;
  std::unique_ptr<libMesh::QGauss> _nodal_scaling_qrule;
};
