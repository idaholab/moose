#pragma once

#include "FVKernel.h"

class FVDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVDiffusion(const InputParameters & params);

protected:
  virtual Real computeQpResidual() override;

  const MaterialProperty<Real> & _coeff_left;
  const MaterialProperty<Real> & _coeff_right;
};
