#pragma once

#include "ADKernel.h"

/**
 * Kernel implementing the weak form of -div(v * grad(u)) where v is a coupled
 * scalar variable, for AD Jacobian testing.
 */
class ADCoupledScalarDiffusion : public ADKernel
{
public:
  static InputParameters validParams();
  ADCoupledScalarDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const ADVariableValue & _v;
};
