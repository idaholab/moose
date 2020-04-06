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

/**
 * Darcy advective flux for a fully-saturated,
 * single phase, single component fluid.
 * No upwinding or relative-permeability is used.
 */
class PorousFlowFullySaturatedDarcyBase : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedDarcyBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * The mobility of the fluid = density / viscosity
   */
  virtual Real mobility() const;

  /**
   * The derivative of the mobility with respect to the PorousFlow variable pvar
   * @param pvar Take the derivative with respect to this PorousFlow variable
   */
  virtual Real dmobility(unsigned pvar) const;

  /// If true then the mobility contains the fluid density, otherwise it doesn't
  const bool _multiply_by_density;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _density;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real>>> & _ddensity_dvar;

  /// Viscosity of the fluid at the qp
  const MaterialProperty<std::vector<Real>> & _viscosity;

  /// Derivative of the fluid viscosity  wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dviscosity_dvar;

  /// Quadpoint pore pressure in each phase
  const MaterialProperty<std::vector<Real>> & _pp;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Gravity pointing downwards
  const RealVectorValue _gravity;

  /// Flag to check whether permeabiity derivatives are non-zero
  const bool _perm_derivs;
};
