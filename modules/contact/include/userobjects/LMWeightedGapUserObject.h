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
  virtual void reinit() override {}
  virtual Real getNormalContactPressure(const Node * const /*node*/) const override;

  virtual void initialize() override;
  virtual void finalize() override;
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

  /// Cache for the D-scaled normal contact pressure returned by contactPressure()
  mutable ADVariableValue _scaled_contact_pressure;
};

inline const std::unordered_map<const DofObject *, std::array<Real, 2>> &
LMWeightedGapUserObject::dofToDerivedC() const
{
  return _dof_to_derived_c;
}
