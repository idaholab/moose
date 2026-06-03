#pragma once

#include "ADKernelScalarBase.h"

/**
 * Contributes a scalar variable residual equal to the integral over the domain of
 * -1 / (1/(1+u) + 1/(1 + grad(u)*grad(u))), where u is the kernel's field variable.
 * No contribution is made to the field equation.
 */
class ADCoupledFieldScalar : public ADKernelScalarBase
{
public:
  static InputParameters validParams();
  ADCoupledFieldScalar(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;
  virtual ADReal computeScalarQpResidual() override;
};
