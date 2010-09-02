/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef KERNELSECOND_H
#define KERNELSECOND_H

// local includes
#include "Kernel.h"

//Forward Declarations
class KernelSecond;

template<>
InputParameters validParams<KernelSecond>();

/** 
 * The KernelSecond class is responsible for calculating the residuals in form:
 *
 *  JxW[_qp] * _tensor[_qp] * _second_test[_i][_qp]
 * 
 */
class KernelSecond : public Kernel
{
public:
  /** 
   * Factory constructor initializes all internal references needed for residual computation.
   * 
   * @param name The name of this kernel.
   * @param moose_system The moose_system this kernel is associated with
   * @param parameters The parameters object for holding additional parameters for kernels and derived kernels
   */
  KernelSecond(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~KernelSecond();

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

protected:
  /**
   * Called before forming the residual for an element
   */
  virtual void precalculateResidual() = 0;

  virtual Real computeQpResidual();

  MooseArray<Real> _value;
};

#endif //KERNELVALUE_H
