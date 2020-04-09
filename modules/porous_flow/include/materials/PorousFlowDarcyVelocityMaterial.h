//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 */
class PorousFlowDarcyVelocityMaterial : public PorousFlowMaterial
{
public:
  static InputParameters validParams();

  PorousFlowDarcyVelocityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of PorousFlow variables
  const unsigned int _num_var;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Fluid density for each phase
  const MaterialProperty<std::vector<Real>> & _fluid_density;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> & _fluid_viscosity;

  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgradvar;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// Gravity
  const RealVectorValue _gravity;

  /**
   * Computed Darcy velocity of each phase.
   * _darcy_velocity[_qp][ph](i) = i^th component of velocity of phase ph at quadpoint _qp
   */
  MaterialProperty<std::vector<RealVectorValue>> & _darcy_velocity;

  /**
   * _ddarcy_velocity_dvar[_qp][ph][v](i) =
   * d(i_th component of velocity of phase ph at quadpoint _qp)/d(PorousFlow variable v)
   */
  MaterialProperty<std::vector<std::vector<RealVectorValue>>> & _ddarcy_velocity_dvar;

  /**
   * _ddarcy_velocity_dgradvar[_qp][ph][j][v](i)
   *   = d(i_th component of Darcy velocity of phase ph at quadpoint _qp) /
   *          d(j_th component of grad(PorousFlow variable v))
   * In an advection Kernel, grad_test * _darcy_velocity[ph] * u, the contribution to the Jacobian
   * is grad_test * (_ddarcy_velocity_dgradvar[_qp][ph][j][v] * _grad_phi[_j][_qp](j)) * u
   */
  MaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>> &
      _ddarcy_velocity_dgradvar;
};
