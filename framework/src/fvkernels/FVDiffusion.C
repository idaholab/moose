
#include "FVDiffusion.h"

registerMooseObject("MooseApp", FVDiffusion);

InputParameters
FVDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  return params;
}

FVDiffusion::FVDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _coeff_left(getMaterialProperty<Real>("coeff")),
    _coeff_right(getNeighborMaterialProperty<Real>("coeff")){};

Real
FVDiffusion::computeQpResidual()
{
  Real dudn = (_u_right[_qp] - _u_left[_qp]) /
              (_face_info->rightCentroid() - _face_info->leftCentroid()).norm();
  auto grad_u_interface = _normal * dudn;
  Real k = (_coeff_left[_qp] + _coeff_right[_qp]) / 2;
  return _normal * k * grad_u_interface;
}
