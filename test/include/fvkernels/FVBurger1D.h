#pragma once

#include "FVKernel.h"

class FVBurger1D : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVBurger1D(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
};
