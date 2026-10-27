//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "PorousFlowDictator.h"
#include "RankTwoTensor.h"

/**
 * Dispersive flux of component k in fluid phase alpha. Includes the effects
 * of both molecular diffusion and hydrodynamic dispersion.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowDispersiveFluxTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowDispersiveFluxTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Derivative of the residual with respect to the PorousFlow variable
   * with variable number jvar. Only used for the non-AD path.
   */
  Real computeQpJac(unsigned int jvar) const;

  /// Fluid density for each phase (at the qp)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density_qp;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_qp_dvar;

  /// Gradient of mass fraction of each component in each phase
  const GenericMaterialProperty<std::vector<std::vector<RealGradient>>, is_ad> & _grad_mass_frac;

  /// Derivative of mass fraction wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_frac_dvar;

  /// Porosity at the qps
  const GenericMaterialProperty<Real, is_ad> & _porosity_qp;

  /// Derivative of porosity wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<Real>> * const _dporosity_qp_dvar;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _tortuosity;

  /// Derivative of tortuosity wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dtortuosity_dvar;

  /// Diffusion coefficients of component k in fluid phase alpha
  const MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;

  /// Derivative of the diffusion coefficients wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const
      _ddiffusion_coeff_dvar;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Index of the fluid component that this kernel acts on
  const unsigned int _fluid_component;

  /// The number of fluid phases
  const unsigned int _num_phases;

  /// Identity tensor (generic type to support AD arithmetic)
  const GenericRankTwoTensor<is_ad> _identity_tensor;

  /// Relative permeability of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _relative_permeability;

  /// Derivative of relative permeability wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<Real>>> * const _drelative_permeability_dvar;

  /// Viscosity of each component in each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_viscosity;

  /// Derivative of viscosity wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_dvar;

  /// Permeability of porous material
  const GenericMaterialProperty<RealTensorValue, is_ad> & _permeability;

  /// Derivative of permeability wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<RealTensorValue>> * const _dpermeability_dvar;

  /// d(permeability)/d(grad(PorousFlow variable)) -- null for AD
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> * const _dpermeability_dgradvar;

  /// Gradient of the pore pressure in each phase
  const GenericMaterialProperty<std::vector<RealGradient>, is_ad> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables) -- null for AD
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables -- null for AD
  const MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgrad_p_dvar;

  /// Gravitational acceleration
  const RealVectorValue _gravity;

  /// Longitudinal dispersivity for each phase
  const std::vector<Real> _disp_long;

  /// Transverse dispersivity for each phase
  const std::vector<Real> _disp_trans;

  /// Flag to check whether permeability derivatives are non-zero
  const bool _perm_derivs;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowDispersiveFluxTempl<false> PorousFlowDispersiveFlux;
typedef PorousFlowDispersiveFluxTempl<true> ADPorousFlowDispersiveFlux;
