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
#include "Material.h"

/**
 * Kernel that allows to construct wrong jacobians, by multiplying a diffusion
 * kernel jacobian and/or residual with an arbitrary prefactor
 */
class WrongJacobianDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  WrongJacobianDiffusion(const InputParameters & parameters);

protected:
  /**
   * Compute the correct diffusion residual with an arbitrary prefactor applied
   */
  virtual Real computeQpResidual() override;

  /**
   * Compute the correct diffusion on-diagonal Jacobian with an arbitrary prefactor applied
   */
  virtual Real computeQpJacobian() override;

  /**
   * Set a constant off-diagonal Jacobian
   */
  virtual Real computeQpOffDiagJacobian(unsigned int) override;

private:
  /// prefactor of the Residual
  Real _rfactor;

  /// prefactor of the Jacobian
  Real _jfactor;
};
