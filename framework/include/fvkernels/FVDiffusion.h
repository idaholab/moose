#pragma once

#include "FVFluxKernel.h"

class FVDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVDiffusion(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _coeff_left;
  const ADMaterialProperty<Real> & _coeff_right;
};
