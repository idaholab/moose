
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
    _coeff_left(getADMaterialProperty<Real>("coeff")),
    _coeff_right(getNeighborADMaterialProperty<Real>("coeff")){};

ADReal
FVDiffusion::computeQpResidual()
{
  auto dudn = gradUDotNormal();
  ADReal k = (_coeff_left[_qp] + _coeff_right[_qp]) / 2;
  return -1 * k * dudn;
}
