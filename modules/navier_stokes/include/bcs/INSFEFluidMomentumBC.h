//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidIntegratedBCBase.h"
#include "Function.h"

/**
 * Specifies flow of momentum out of a boundary. For the convection term, the formula could be
 * in the conservative form, therefore requires integration by parts, or in the primitive form,
 * therefore integration by parts is not applied.
 * For the pressure gradient term, integration by parts is also optional.
 * Either a pressure or velocity boundary condition may be supplied
 * through functions; branch pressure and density may be supplied, instead of a pressure function,
 * for gravity head calculations
 */
class INSFEFluidMomentumBC : public INSFEFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  INSFEFluidMomentumBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// Whether conservative form to be used for the convection term
  const bool _conservative_form;
  /// Whether integration by parts to be used for the pressure gradient term
  const bool _p_int_by_parts;
  /// The component (x=0, y=1, z=2) of the momentum equation this kernel is applied
  const unsigned _component;
  /// Fluid dynamic viscosity
  const MaterialProperty<Real> & _mu;
  /// Turbulent viscosity
  const MaterialProperty<Real> & _mu_t;
  /// Whether boundary pressure is specified
  const bool _has_pbc;
  /// Whether boundary velocity is specified
  const bool _has_vbc;
  /// The function that specifies the boundary pressure
  const Function * _p_fn;
  /// The function that specifies the boundary velocity
  const Function * _v_fn;

  // variables for branch pressure and density
  /// Whether a (SAM) branch component is connected
  bool _has_pbranch;
  /// The pressure of the connected branch component
  const VariableValue & _p_branch;
  /// The var number of the branch pressure
  unsigned int _p_branch_var_number;
  /// The fluid density of the connected branch component
  const VariableValue & _rho_branch;
  /// Gravity vector
  RealVectorValue _vec_g;
  /// The location of the center (a reference point) of the connected branch
  Point _branch_center;
};
