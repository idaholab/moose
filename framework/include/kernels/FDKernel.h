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

#ifndef FDKERNEL_H
#define FDKERNEL_H

#include "Kernel.h"
#include "MooseObject.h"

class FDKernel;

template<>
InputParameters validParams<FDKernel>();

class FDKernel :
  public Kernel
{
public:
  FDKernel(const std::string & name, InputParameters parameters);

  virtual ~FDKernel(){};

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
  virtual DenseVector<Number>
    perturbedResidual(unsigned int ivar, unsigned int i, Real perturbation_scale, Real& perturbation);

  Real _scale;
};

#endif /* FDKERNEL_H */
