#ifndef ONEDARTIFICIALDISSIPATION_H
#define ONEDARTIFICIALDISSIPATION_H

#include "Kernel.h"

class OneDArtificialDissipation;

template <>
InputParameters validParams<OneDArtificialDissipation>();

/**
 *
 */
class OneDArtificialDissipation : public Kernel
{
public:
  OneDArtificialDissipation(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// The gradient of the coupled velocity aux of this phase
  const VariableValue & _area;
  const VariableValue & _U;
  const VariableGradient & _grad_U;
  const VariableValue & _alpha;
  const VariableGradient & _grad_alpha;
  const MaterialProperty<Real> & _coef;
};

#endif /* ONEDARTIFICIALDISSIPATION_H */
