//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowLumpedKernelBase.h"
#include "PorousFlowDictator.h"

/**
 * Kernel = _decay_rate * masscomponent
 * where mass_component =
 * porosity*sum_phases(density_phase*saturation_phase*massfrac_phase^component)
 * It is lumped to the nodes
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowMassRadioactiveDecayTempl : public PorousFlowLumpedKernelBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowMassRadioactiveDecayTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Derivative of residual wrt PorousFlow variable pvar (non-AD path only)
  Real computeQpJac(unsigned int pvar);

  /// The decay rate
  const Real _decay_rate;

  /// The fluid component index
  const unsigned int _fluid_component;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the Variable for this Kernel is a PorousFlow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// Whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// Porosity at the nodes (AD or non-AD depending on is_ad)
  const GenericMaterialProperty<Real, is_ad> & _porosity;

  /// d(porosity)/d(PorousFlow variable) at nodes — null for AD path
  const MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) at qps — null for AD path
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /// The nearest qp to the node (always non-AD)
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Nodal fluid density (AD or non-AD)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density;

  /// d(nodal fluid density)/d(PorousFlow variable) — null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Nodal fluid saturation (AD or non-AD)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_saturation_nodal;

  /// d(nodal fluid saturation)/d(PorousFlow variable) — null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_saturation_nodal_dvar;

  /// Nodal mass fraction (AD or non-AD)
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_frac;

  /// d(nodal mass fraction)/d(PorousFlow variable) — null for AD path
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_frac_dvar;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowMassRadioactiveDecayTempl<false> PorousFlowMassRadioactiveDecay;
typedef PorousFlowMassRadioactiveDecayTempl<true> ADPorousFlowMassRadioactiveDecay;
