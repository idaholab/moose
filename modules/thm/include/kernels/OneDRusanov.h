#ifndef ONEDRUSANOV_H
#define ONEDRUSANOV_H

#include "Kernel.h"

class OneDRusanov;

template <>
InputParameters validParams<OneDRusanov>();

/**
 *
 */
class OneDRusanov : public Kernel
{
public:
  OneDRusanov(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real coef();

  const MaterialProperty<Real> & _c;
  const VariableValue & _vel;
  const VariableGradient & _velocity_grad;
};

#endif /* ONEDRUSANOV_H */
