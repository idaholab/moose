//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FDKERNEL_H
#define FDKERNEL_H

#include "Kernel.h"

class FDKernel;

template <>
InputParameters validParams<FDKernel>();

class FDKernel : public Kernel
{
public:
  FDKernel(const InputParameters & parameters);

  virtual void computeJacobian();
  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

protected:
  /**
   * Computes the residual when the current state of j-th variable
   * at element node i is perturbed by perturbation.
   * With perturbation = 0.0 we have the unperturbed residual.
   */
  virtual DenseVector<Number> perturbedResidual(unsigned int ivar,
                                                unsigned int i,
                                                Real perturbation_scale,
                                                Real & perturbation);

  Real _scale;
};

#endif /* FDKERNEL_H */
