//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DGKERNEL_H
#define DGKERNEL_H

#include "DGKernelBase.h"

class DGKernel;

template <>
InputParameters validParams<DGKernel>();

/**
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 */
class DGKernel : public DGKernelBase
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   *
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  DGKernel(const InputParameters & parameters);

  virtual ~DGKernel();

  /**
   * Computes the residual for this element or the neighbor
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) override;

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) override;

  /**
   * Computes the element-element off-diagonal Jacobian
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                               unsigned int jvar) override;

protected:
  /**
   * This is the virtual that derived classes should override for computing the residual on
   * neighboring element.
   */
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on
   * neighboring element.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the off-diag Jacobian.
   */
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);
};

#endif // DGKERNEL_H
