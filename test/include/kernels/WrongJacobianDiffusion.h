/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
