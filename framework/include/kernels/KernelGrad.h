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

//Forward Declarations
class KernelGrad;

template<>
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
   * @param name The name of this kernel.
   * @param moose_system The moose_system this kernel is associated with
   * @param parameters The parameters object for holding additional parameters for kernels and derived kernels
   */
  KernelGrad(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~KernelGrad();

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

  /** 
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar);

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual RealGradient precomputeQpResidual() = 0;

  /**
   * Called before forming the jacobian for an element
   */
  virtual RealGradient precomputeQpJacobian(){ return RealGradient(); }

  virtual Real computeQpResidual();

  RealGradient _value;
};

#endif //KERNELVALUE_H
