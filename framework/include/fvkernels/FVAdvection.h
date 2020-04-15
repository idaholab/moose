#pragma once

#include "FVKernel.h"

class FVAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  ADRealVectorValue _velocity;
};
