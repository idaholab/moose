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

#ifndef KERNELVALUE_H
#define KERNELVALUE_H

// local includes
#include "Kernel.h"

// Forward Declarations
class KernelValue;

template <>
InputParameters validParams<KernelValue>();

/**
 * The KernelValue class is responsible for calculating the residuals in form:
 *
 *  JxW[_qp] * _value[_qp] * _test[_i][_qp]
 *
 */
class KernelValue : public Kernel
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  KernelValue(const InputParameters & parameters);

  virtual void computeResidual() override;

  virtual void computeJacobian() override;

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual Real precomputeQpResidual() = 0;

  /**
   * Called before forming the jacobian for an element
   */
  virtual Real precomputeQpJacobian();

  virtual Real computeQpResidual() override;
};

#endif // KERNELVALUE_H
