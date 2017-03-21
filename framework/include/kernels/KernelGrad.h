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

#ifndef KERNELGRAD_H
#define KERNELGRAD_H

// local includes
#include "Kernel.h"

// Forward Declarations
class KernelGrad;

template <>
InputParameters validParams<KernelGrad>();

/**
 * The KernelGrad class is responsible for calculating the residuals in form:
 *
 *  JxW[_qp] * _vector[_qp] * _grad_test[_i][_qp]
 *
 */
class KernelGrad : public Kernel
{
public:
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

#endif // KERNELGRAD_H
