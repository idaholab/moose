#pragma once

#include "ADKernelValue.h"

/**
 * Field kernel computing exp(u - 1) for the Keyes MSPIN unit test.
 */
class ADExpUKernel : public ADKernelValue
{
public:
  static InputParameters validParams();
  ADExpUKernel(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;
};
