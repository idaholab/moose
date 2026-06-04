#pragma once

#include "ElementADScalarKernel.h"

/**
 * Contributes a scalar variable residual equal to the integral over the domain of
 * -1 / (1/(1+u) + 1/(1 + grad(u)*grad(u))), where u is a coupled field variable.
 */
class ADCoupledFieldScalar : public ElementADScalarKernel
{
public:
  static InputParameters validParams();
  ADCoupledFieldScalar(const InputParameters & parameters);

protected:
  ADReal computeScalarQpResidual() override;

  const ADVariableValue & _u;
  const ADVariableGradient & _grad_u;
};
