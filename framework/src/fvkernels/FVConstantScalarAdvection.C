
#include "FVConstantScalarAdvection.h"

registerADMooseObject("MooseApp", FVConstantScalarAdvection);

template <ComputeStage compute_stage>
InputParameters
FVConstantScalarAdvection<compute_stage>::validParams()
{
  InputParameters params = FVFluxKernel<compute_stage>::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

template <ComputeStage compute_stage>
FVConstantScalarAdvection<compute_stage>::FVConstantScalarAdvection(const InputParameters & params)
  : FVFluxKernel<compute_stage>(params),
    _velocity(getParam<RealVectorValue>("velocity"))
{};

template <ComputeStage compute_stage>
ADReal
FVConstantScalarAdvection<compute_stage>::computeQpResidual()
{
  ADReal r = 0;
  if (_velocity * _normal > 0)
    r = _normal * _velocity * _u_left[_qp];
  else
    r = _normal * _velocity * _u_right[_qp];
  return r;
}
