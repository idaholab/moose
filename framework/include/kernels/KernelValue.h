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
  KernelValue(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~KernelValue();

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
