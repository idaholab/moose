#pragma once

#include "ADKernelValue.h"

/**
 * Field kernel for the Keyes MSPIN test coupled source term.
 */
class ADCoupledFieldKernel : public ADKernelValue
{
public:
  static InputParameters validParams();
  ADCoupledFieldKernel(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADVariableValue & _coupled_u;
  const ADVariableGradient & _grad_coupled_u;
};
