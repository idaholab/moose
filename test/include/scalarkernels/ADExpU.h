#pragma once

#include "ADScalarKernel.h"

/**
 * Scalar kernel computing exp(u - 1) for AD Jacobian testing.
 */
class ADExpU : public ADScalarKernel
{
public:
  static InputParameters validParams();
  ADExpU(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;
};
