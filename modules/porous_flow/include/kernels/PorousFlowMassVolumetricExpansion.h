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
 * Kernel = mass_component * rate_of_solid_volumetric_expansion
 * where mass_component = porosity * sum_phases(density_phase * saturation_phase *
 * massfrac_phase^component) It is lumped to the nodes. If multiply_by_density = false then
 * density_phase is not included in the above sum.
 */
template <bool is_ad>
class PorousFlowMassVolumetricExpansionTempl : public PorousFlowLumpedKernelBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowMassVolumetricExpansionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

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

  /// Whether to multiply by density
  const bool _multiply_by_density;

  /// Porosity at the nodes
  const GenericMaterialProperty<Real, is_ad> & _porosity;

  /// d(porosity)/d(PorousFlow variable) at the nodes
  const MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) at the qps
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /// The nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Nodal fluid density
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _fluid_density;

  /// d(nodal fluid density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Nodal fluid saturation
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_saturation;

  /// d(nodal fluid saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_saturation_dvar;

  /// Nodal mass fraction
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_frac;

  /// d(nodal mass fraction)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_frac_dvar;

  /// Strain rate
  const GenericMaterialProperty<Real, is_ad> & _strain_rate_qp;

  /// d(strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> * const _dstrain_rate_qp_dvar;

  /**
   * Derivative of mass part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the mass part of the residual wrt this variable number
   */
  Real computedMassQpJac(unsigned int jvar) const;

  /**
   * Derivative of volumetric-strain part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the volumetric-strain part of the residual wrt this variable
   * number
   */
  Real computedVolQpJac(unsigned int jvar) const;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowMassVolumetricExpansionTempl<false> PorousFlowMassVolumetricExpansion;
typedef PorousFlowMassVolumetricExpansionTempl<true> ADPorousFlowMassVolumetricExpansion;
