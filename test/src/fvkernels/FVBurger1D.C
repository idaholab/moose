
#include "FVBurger1D.h"

registerADMooseObject("MooseTestApp", FVBurger1D);

template <ComputeStage compute_stage>
InputParameters
FVBurger1D<compute_stage>::validParams()
{
  InputParameters params = FVFluxKernel<compute_stage>::validParams();
  return params;
}

template <ComputeStage compute_stage>
FVBurger1D<compute_stage>::FVBurger1D(const InputParameters & params)
  : FVFluxKernel<compute_stage>(params)
{
}

template <ComputeStage compute_stage>
ADReal
FVBurger1D<compute_stage>::computeQpResidual()
{
  mooseAssert(_face_info->leftElem().dim() == 1, "FVBurger1D works only in 1D");
  ADReal r = 0;
  ADReal u_av = 0.5 * (_u_left[_qp] + _u_right[_qp]);
  if (u_av * _normal(0) > 0)
    r = 0.5 * _u_left[_qp] * _u_left[_qp] * _normal(0);
  else
    r = 0.5 * _u_right[_qp] * _u_right[_qp] * _normal(0);
  return r;
}
