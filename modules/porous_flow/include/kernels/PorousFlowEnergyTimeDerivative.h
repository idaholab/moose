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
 * Kernel = (heat_energy - heat_energy_old)/dt
 * It is lumped to the nodes
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowEnergyTimeDerivativeTempl : public PorousFlowLumpedKernelBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowEnergyTimeDerivativeTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Derivative of residual with respect to PorousFlow variable number pvar (non-AD path only)
   * @param pvar take the derivative of the residual wrt this PorousFlow variable
   */
  Real computeQpJac(unsigned int pvar) const;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the Variable for this Kernel is a PorousFlow variable according to the Dictator
  const bool _var_is_porflow_var;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// Whether _num_phases > 0 (ie. there is a fluid present)
  const bool _fluid_present;

  /// Whether the porosity uses the volumetric strain at the closest quadpoint
  const bool _strain_at_nearest_qp;

  /// base name used in the Tensor Mechanics strain calculator
  const std::string _base_name;

  /// Whether there is a Material called _base_name_total_strain
  const bool _has_total_strain;

  /// Old value of total strain calculated by a Tensor Mechanics strain calculator, if it exists, otherwise nullptr
  const MaterialProperty<RankTwoTensor> * const _total_strain_old;

  /// Porosity at the nodes (AD or non-AD depending on is_ad)
  const GenericMaterialProperty<Real, is_ad> & _porosity;

  /// Old value of porosity (always non-AD)
  const MaterialProperty<Real> & _porosity_old;

  /// d(porosity)/d(PorousFlow variable) at nodes — null for AD path
  const MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) at qps — null for AD path
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /// The nearest qp to the node (always non-AD)
  const MaterialProperty<unsigned int> * const _nearest_qp;

  /// Nodal rock energy density (AD or non-AD)
  const GenericMaterialProperty<Real, is_ad> & _rock_energy_nodal;

  /// Old value of nodal rock energy density (always non-AD)
  const MaterialProperty<Real> & _rock_energy_nodal_old;

  /// d(nodal rock energy density)/d(PorousFlow variable) — null for AD path
  const MaterialProperty<std::vector<Real>> * const _drock_energy_nodal_dvar;

  /// Nodal fluid density (AD or non-AD); null if !_fluid_present
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _fluid_density;

  /// Old value of nodal fluid density (always non-AD); null if !_fluid_present
  const MaterialProperty<std::vector<Real>> * const _fluid_density_old;

  /// d(nodal fluid density)/d(PorousFlow variable) — null for AD path or if !_fluid_present
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Nodal fluid saturation (AD or non-AD); null if !_fluid_present
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _fluid_saturation_nodal;

  /// Old value of fluid saturation (always non-AD); null if !_fluid_present
  const MaterialProperty<std::vector<Real>> * const _fluid_saturation_nodal_old;

  /// d(nodal fluid saturation)/d(PorousFlow variable) — null for AD path or if !_fluid_present
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_saturation_nodal_dvar;

  /// Internal energy of the phases, evaluated at the nodes (AD or non-AD); null if !_fluid_present
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _energy_nodal;

  /// Old value of internal energy of the phases, evaluated at the nodes (always non-AD); null if !_fluid_present
  const MaterialProperty<std::vector<Real>> * const _energy_nodal_old;

  /// d(internal energy)/d(PorousFlow variable) — null for AD path or if !_fluid_present
  const MaterialProperty<std::vector<std::vector<Real>>> * const _denergy_nodal_dvar;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowEnergyTimeDerivativeTempl<false> PorousFlowEnergyTimeDerivative;
typedef PorousFlowEnergyTimeDerivativeTempl<true> ADPorousFlowEnergyTimeDerivative;
