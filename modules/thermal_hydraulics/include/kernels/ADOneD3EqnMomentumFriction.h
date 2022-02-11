//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Computes wall friction term for single phase flow
 *
 * See RELAP-7 Theory Manual, pg. 71, Equation (230) {eq:wall_friction_force_2phase}
 */
class ADOneD3EqnMomentumFriction : public ADKernel
{
public:
  ADOneD3EqnMomentumFriction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// area
  const ADVariableValue & _A;

  /// Hydraulic diameter
  const ADMaterialProperty<Real> & _D_h;

  /// Density
  const ADMaterialProperty<Real> & _rho;

  /// velocity
  const ADMaterialProperty<Real> & _vel;

  /// Darcy friction factor
  const ADMaterialProperty<Real> & _f_D;

public:
  static InputParameters validParams();
};
