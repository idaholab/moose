
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
  std::cout << "***** computeQpResidual left=" << _face_info->leftElem().id() << ", right=";
  if (_face_info->rightElemPtr())
    std::cout << _face_info->rightElem().id();
  else
    std::cout << "GHOST";
  std::cout << "\n";
  std::cout << "    normal: " << _face_info->normal() << "\n";
  std::cout << "    face centroid: " << _face_info->faceCentroid() << "\n";
  std::cout << "    u_left: " << _u_left[0] << "\n";
  std::cout << "    u_right: " << _u_right[0] << "\n";
  std::cout << "    grad_u_interface_x: " << grad_u_interface(0) << "\n";
  std::cout << "    grad_u_interface_y: " << grad_u_interface(1) << "\n";
  std::cout << "    grad_u_interface_z: " << grad_u_interface(2) << "\n";
  std::cout << "    residual: " << r << "\n";
  return r;
}

template <ComputeStage compute_stage>
ADReal
FVDiffusion<compute_stage>::computeQpJacobian()
{
  return 0;
}
