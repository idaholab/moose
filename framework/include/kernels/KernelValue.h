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
  virtual void computeJacobian(int i, int j);

  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar);

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
