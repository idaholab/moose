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
 * Kernel = energy_density * rate_of_solid_volumetric_expansion
 * where energy_density = (1 - porosity) * rock_energy_density
 *  + porosity * sum_phases(density_phase * saturation_phase * internal_energy_phase)
 * Note: the energy density is lumped to the nodes
 */
template <bool is_ad>
class PorousFlowHeatVolumetricExpansionTempl : public PorousFlowLumpedKernelBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowHeatVolumetricExpansionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the Variable for this Kernel is a PorousFlow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// Whether fluid is present
  const bool _fluid_present;

  /// Whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// Porosity at the nodes
  const GenericMaterialProperty<Real, is_ad> & _porosity;

  /// d(porosity)/d(PorousFlow variable) at the nodes
  const MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) at the qps
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /// The nearest qp to the node
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Nodal rock energy density
  const GenericMaterialProperty<Real, is_ad> & _rock_energy_nodal;

  /// d(nodal rock energy density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _drock_energy_nodal_dvar;

  /// Nodal fluid density
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _fluid_density;

  /// d(nodal fluid density)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Nodal fluid saturation
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _fluid_saturation_nodal;

  /// d(nodal fluid saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_saturation_nodal_dvar;

  /// Nodal fluid internal energy
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _energy_nodal;

  /// d(nodal fluid internal energy)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _denergy_nodal_dvar;

  /// Strain rate
  const GenericMaterialProperty<Real, is_ad> & _strain_rate_qp;

  /// d(strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> * const _dstrain_rate_qp_dvar;

  /**
   * Derivative of energy part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the energy part of the residual wrt this variable number
   */
  Real computedEnergyQpJac(unsigned int jvar);

  /**
   * Derivative of volumetric-strain part of the residual with respect to the Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the volumetric-strain part of the residual wrt this variable
   * number
   */
  Real computedVolQpJac(unsigned int jvar);

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowHeatVolumetricExpansionTempl<false> PorousFlowHeatVolumetricExpansion;
typedef PorousFlowHeatVolumetricExpansionTempl<true> ADPorousFlowHeatVolumetricExpansion;
