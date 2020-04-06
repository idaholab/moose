#pragma once

#include "FVKernel.h"

template <ComputeStage compute_stage>
class FVBurger1D : public FVFluxKernel<compute_stage>
{
public:
  static InputParameters validParams();
  FVBurger1D(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  usingFVFluxKernelMembers;
};
