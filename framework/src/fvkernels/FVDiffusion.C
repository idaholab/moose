
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
  auto grad_u_interface = _normal * dudn;
  ADReal k = (_coeff_left[_qp] + _coeff_right[_qp]) / 2;
  return _normal * -1 * k * grad_u_interface;
}

template <ComputeStage compute_stage>
ADReal
FVDiffusion<compute_stage>::computeQpJacobian()
{
  return 0;
}
