#pragma once

#include "FVKernel.h"

template <ComputeStage compute_stage>
class FVDiffusion : public FVFluxKernel<compute_stage>
{
public:
  static InputParameters validParams();
  FVDiffusion(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  virtual ADReal computeQpJacobian() override;

  const ADMaterialProperty(Real) & _coeff_left;
  const MaterialProperty<Real> & _coeff_right;

  usingFVFluxKernelMembers;
};
