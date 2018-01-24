#ifndef DEFAULTMATPROPCONSUMERKERNEL_H
#define DEFAULTMATPROPCONSUMERKERNEL_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

// Forward declarations
class DefaultMatPropConsumerKernel;

template <>
InputParameters validParams<DefaultMatPropConsumerKernel>();

class DefaultMatPropConsumerKernel : public DerivativeMaterialInterface<Kernel>
{
public:
  DefaultMatPropConsumerKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() { return 0.0; };

  const MaterialProperty<Real> & _prop;
};

#endif // DEFAULTMATPROPCONSUMERKERNEL_H
