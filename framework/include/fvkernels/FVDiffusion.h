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

  const ADMaterialProperty(Real) & _coeff_left;
  const ADMaterialProperty(Real) & _coeff_right;

  usingFVFluxKernelMembers;
};

template <ComputeStage compute_stage>
class FVAdvection : public FVFluxKernel<compute_stage>
{
public:
  static InputParameters validParams();
  FVAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty(RealVectorValue) & _vel_left;
  const ADMaterialProperty(RealVectorValue) & _vel_right;

  usingFVFluxKernelMembers;
};
