
#include "FVDiffusion.h"

registerADMooseObject("MooseApp", FVDiffusion);

template <ComputeStage compute_stage>
InputParameters
FVDiffusion<compute_stage>::validParams()
{
  InputParameters params = FVFluxKernel<compute_stage>::validParams();
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  return params;
}

template <ComputeStage compute_stage>
FVDiffusion<compute_stage>::FVDiffusion(const InputParameters & params)
  : FVFluxKernel<compute_stage>(params),
    _coeff_left(getADMaterialProperty<Real>("coeff")),
    _coeff_right(getNeighborADMaterialProperty<Real>("coeff")){};

template <ComputeStage compute_stage>
ADReal
FVDiffusion<compute_stage>::computeQpResidual()
{
  ADReal dudn = (_u_right[_qp] - _u_left[_qp]) /
                (_face_info->rightCentroid() - _face_info->leftCentroid()).norm();
  ADRealVectorValue grad_u_interface = _normal * dudn;
  ADReal k = (_coeff_left[_qp] + _coeff_right[_qp]) / 2;
  ADReal r = _normal * -1 * k * grad_u_interface;
  return r;
}

registerADMooseObject("MooseApp", FVMatAdvection);

template <ComputeStage compute_stage>
InputParameters
FVMatAdvection<compute_stage>::validParams()
{
  InputParameters params = FVFluxKernel<compute_stage>::validParams();
  params.addRequiredParam<MaterialPropertyName>("vel", "advection velocity");
  return params;
}

template <ComputeStage compute_stage>
FVMatAdvection<compute_stage>::FVMatAdvection(const InputParameters & params)
  : FVFluxKernel<compute_stage>(params),
    _vel_left(getADMaterialProperty<RealVectorValue>("vel")),
    _vel_right(getNeighborADMaterialProperty<RealVectorValue>("vel")){};

template <ComputeStage compute_stage>
ADReal
FVMatAdvection<compute_stage>::computeQpResidual()
{
  auto v_avg = (_vel_left[_qp] + _vel_right[_qp]) * 0.5;
  ADReal r = 0;
  if (v_avg * _normal > 0)
    r = _normal * v_avg * _u_left[_qp];
  else
    r = _normal * v_avg * _u_right[_qp];
  return r;
}
