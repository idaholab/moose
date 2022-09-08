//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

/**
 *  VectorInterfaceKernel that enforces the difference between the perpendicular
 *  vector field components on either side of a boundary based on the electrical
 *  permittivities on either side of the interface as well as the free charge
 *  build-up
 */
class PerpendicularElectricFieldInterface : public VectorInterfaceKernel
{
public:
  static InputParameters validParams();

  PerpendicularElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  /// Free charge on the interface (default = 0)
  Real _free_charge;

  /// Free charge dotted with interface normal
  RealVectorValue _free_charge_dot_n;

  /// Electrical permittivity on the primary side of the boundary
  Real _primary_eps;

  /// Electrical permittivity on the secondary side of the boundary
  Real _secondary_eps;

  /// Perpendicular field component of the solution variable on the primary side of the boundary
  RealVectorValue _u_perp;

  /// Perpendicular field component of the solution variable on the secondary side of the boundary
  RealVectorValue _secondary_perp;

  /// Perpendicular field component of the test function on the primary side of the boundary
  RealVectorValue _phi_u_perp;

  /// Perpendicular field component of the test function on the secondary side of the boundary
  RealVectorValue _phi_secondary_perp;
};
