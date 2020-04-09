//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "PorousFlowDictator.h"
#include "RankTwoTensor.h"

/**
 * Dispersive flux of component k in fluid phase alpha. Includes the effects
 * of both molecular diffusion and hydrodynamic dispersion.
 */
class PorousFlowDispersiveFlux : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowDispersiveFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Derivative of the residual with respect to the PorousFLow Variable
   * with variable number jvar.
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative wrt this variable number
   * @return dResidual_dVar
   */
  Real computeQpJac(unsigned int jvar) const;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _fluid_density_qp;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_qp_dvar;

  /// Gradient of mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _grad_mass_frac;

  /// Derivative of mass fraction wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;

  /// Porosity at the qps
  const MaterialProperty<Real> & _porosity_qp;

  /// Derivative of porosity wrt PorousFlow variables (at the qps)
  const MaterialProperty<std::vector<Real>> & _dporosity_qp_dvar;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  const MaterialProperty<std::vector<Real>> & _tortuosity;

  /// Derivative of tortuosity wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dtortuosity_dvar;

  /// Diffusion coefficients of component k in fluid phase alpha
  const MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;

  /// Derivative of the diffusion coefficients wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _ddiffusion_coeff_dvar;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Index of the fluid component that this kernel acts on
  const unsigned int _fluid_component;

  /// The number of fluid phases
  const unsigned int _num_phases;

  /// Identity tensor
  const RankTwoTensor _identity_tensor;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> & _fluid_viscosity;

  /// Derivative of viscosity wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// Derivative of permeability wrt PorousFlow variables
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// Gravitational acceleration
  const RealVectorValue _gravity;

  /// Longitudinal dispersivity for each phase
  const std::vector<Real> _disp_long;

  /// Transverse dispersivity for each phase
  const std::vector<Real> _disp_trans;

  /// Flag to check whether permeabiity derivatives are non-zero
  const bool _perm_derivs;
};
