#pragma once

#include "FVKernel.h"

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

class FVMatAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVMatAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _vel_left;
  const ADMaterialProperty<RealVectorValue> & _vel_right;
};
