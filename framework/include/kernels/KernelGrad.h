//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "Kernel.h"

/**
 * The KernelGrad class is responsible for calculating the residuals in form:
 *
 *  JxW[_qp] * _vector[_qp] * _grad_test[_i][_qp]
 *
 */
class KernelGrad : public Kernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  KernelGrad(const InputParameters & parameters);

  virtual void computeResidual() override;

  virtual void computeJacobian() override;

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual RealGradient precomputeQpResidual() = 0;

  /**
   * Called before forming the jacobian for an element
   */
  virtual RealGradient precomputeQpJacobian();

  virtual Real computeQpResidual() override;
};
