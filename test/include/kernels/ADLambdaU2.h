#pragma once

#include "ADKernel.h"

/**
 * Kernel computing lambda * u^2 for AD Jacobian testing.
 */
class ADLambdaU2 : public ADKernel
{
public:
  static InputParameters validParams();
  ADLambdaU2(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const Real _lambda;
};
