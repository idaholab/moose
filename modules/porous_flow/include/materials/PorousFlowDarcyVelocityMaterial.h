//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterial.h"

/**
 * Material to calculate the Darcy velocity for all phases
 *
 * Templated on is_ad: the false instantiation also computes the hand-coded derivative
 * properties; the true instantiation carries derivatives through the AD Darcy velocity.
 */
template <bool is_ad>
class PorousFlowDarcyVelocityMaterialTempl : public PorousFlowMaterial
{
public:
  static InputParameters validParams();

  PorousFlowDarcyVelocityMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of PorousFlow variables
  const unsigned int _num_var;

  /// Permeability of porous material
  const GenericMaterialProperty<RealTensorValue, is_ad> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<RealTensorValue>> * const _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable)) -- null for AD path
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> * const _dpermeability_dgradvar;

  /// Fluid density for each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Viscosity of each component in each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_viscosity;

  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_dvar;

  /// Relative permeability of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _drelative_permeability_dvar;

  /// Gradient of the pore pressure in each phase
  const GenericMaterialProperty<std::vector<RealGradient>, is_ad> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dgrad_p_dgradvar;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgrad_p_dvar;

  /// Gravity
  const RealVectorValue _gravity;

  /**
   * Computed Darcy velocity of each phase.
   * _darcy_velocity[_qp][ph](i) = i^th component of velocity of phase ph at quadpoint _qp
   */
  GenericMaterialProperty<std::vector<RealVectorValue>, is_ad> & _darcy_velocity;

  /**
   * _ddarcy_velocity_dvar[_qp][ph][v](i) =
   * d(i_th component of velocity of phase ph at quadpoint _qp)/d(PorousFlow variable v)
   * Null for the AD path.
   */
  MaterialProperty<std::vector<std::vector<RealVectorValue>>> * const _ddarcy_velocity_dvar;

  /**
   * _ddarcy_velocity_dgradvar[_qp][ph][j][v](i)
   *   = d(i_th component of Darcy velocity of phase ph at quadpoint _qp) /
   *          d(j_th component of grad(PorousFlow variable v))
   * In an advection Kernel, grad_test * _darcy_velocity[ph] * u, the contribution to the Jacobian
   * is grad_test * (_ddarcy_velocity_dgradvar[_qp][ph][j][v] * _grad_phi[_j][_qp](j)) * u
   * Null for the AD path.
   */
  MaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>> * const
      _ddarcy_velocity_dgradvar;
};

typedef PorousFlowDarcyVelocityMaterialTempl<false> PorousFlowDarcyVelocityMaterial;
typedef PorousFlowDarcyVelocityMaterialTempl<true> ADPorousFlowDarcyVelocityMaterial;
