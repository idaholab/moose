//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WRONGJACOBIANDIFFUSION_H
#define WRONGJACOBIANDIFFUSION_H

#include "Kernel.h"
#include "Material.h"

// Forward Declarations
class WrongJacobianDiffusion;

template <>
InputParameters validParams<WrongJacobianDiffusion>();

/**
 * Kernel that allows to construct wrong jacobians, by multiplying a diffusion
 * kernel jacobian and/or residual with an arbitrary prefactor
 */
class WrongJacobianDiffusion : public Kernel
{
public:
  WrongJacobianDiffusion(const InputParameters & parameters);

protected:
  /**
   * Compute the correct diffusion residual with an arbitrary prefactor applied
   */
  virtual Real computeQpResidual();

  /**
   * Compute the correct diffusion on-diagonal Jacobian with an arbitrary prefactor applied
   */
  virtual Real computeQpJacobian();

  /**
   * Set a constant off-diagonal Jacobian
   */
  virtual Real computeQpOffDiagJacobian(unsigned int);

private:
  /// prefactor of the Residual
  Real _rfactor;

  /// prefactor of the Jacobian
  Real _jfactor;
};

#endif // WRONGJACOBIANDIFFUSION_H
