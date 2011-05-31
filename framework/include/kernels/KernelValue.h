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

//Forward Declarations
class KernelValue;

template<>
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
   * @param name The name of this kernel.
   * @param moose_system The moose_system this kernel is associated with
   * @param parameters The parameters object for holding additional parameters for kernels and derived kernels
   */
  KernelValue(const std::string & name, InputParameters parameters);
  
  virtual ~KernelValue();

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

  /** 
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual Real precomputeQpResidual() = 0;

  /**
   * Called before forming the jacobian for an element
   */
  virtual Real precomputeQpJacobian(){ return 0; }

  virtual Real computeQpResidual();

  Real _value;
};

#endif //KERNELVALUE_H
