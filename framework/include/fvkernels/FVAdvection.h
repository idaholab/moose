#pragma once

#include "FVKernel.h"

template <ComputeStage compute_stage>
class FVAdvection : public FVFluxKernel<compute_stage>
{
public:
  static InputParameters validParams();
  FVAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  ADRealVectorValue _velocity;

  usingFVFluxKernelMembers;
};
