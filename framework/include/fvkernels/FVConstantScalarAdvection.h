#pragma once

#include "FVKernel.h"

template <ComputeStage compute_stage>
class FVConstantScalarAdvection : public FVFluxKernel<compute_stage>
{
public:
  static InputParameters validParams();
  FVConstantScalarAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  ADRealVectorValue _velocity;

  usingFVFluxKernelMembers;
};
