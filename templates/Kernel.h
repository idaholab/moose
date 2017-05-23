
#ifndef TMPLKERNEL_H
#define TMPLKERNEL_H

#include "Kernel.h"

class TmplKernel;

template<>
InputParameters validParams<TmplKernel>();

class TmplKernel : public Kernel
{
public:
  TmplKernel(const InputParameters & parameters);

protected:

  /// Compute this kernel's contribution to the residual at the current
  /// quadrature point.
  virtual Real computeQpResidual();

  /// Compute this kernel's contribution to the Jacobian at the current
  /// quadrature point.
  virtual Real computeQpJacobian();

  /// Implement if you need to compute an off-diagonal Jacobian component.
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Following methods can be used by kernels that need to perform a
  /// per-element calculation:
  virtual void precalculateResidual();
  virtual void precalculateJacobian();
  virtual void precalculateOffDiagJacobian(unsigned int /* jvar */);
};

#endif
