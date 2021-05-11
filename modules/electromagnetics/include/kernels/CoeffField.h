#pragma once

#include "ADKernel.h"

class CoeffField : public ADKernel
{
public:
  static InputParameters validParams();

  CoeffField(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  Real _coefficient;

  const Function & _func;
};
