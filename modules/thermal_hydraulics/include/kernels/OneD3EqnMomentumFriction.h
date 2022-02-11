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
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Computes wall friction term for single phase flow
 *
 * See RELAP-7 Theory Manual, pg. 71, Equation (230) {eq:wall_friction_force_2phase}
 */
class OneD3EqnMomentumFriction : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnMomentumFriction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// area
  const VariableValue & _A;

  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;

  /// Density
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_darhoA;

  /// velocity
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// Darcy friction factor
  const MaterialProperty<Real> & _f_D;
  const MaterialProperty<Real> & _df_D_darhoA;
  const MaterialProperty<Real> & _df_D_darhouA;
  const MaterialProperty<Real> & _df_D_darhoEA;

  unsigned int _arhoA_var_number;
  unsigned int _arhouA_var_number;
  unsigned int _arhoEA_var_number;

public:
  static InputParameters validParams();
};
