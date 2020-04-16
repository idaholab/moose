
#include "FVMatAdvection.h"

registerADMooseObject("MooseApp", FVMatAdvection);

InputParameters
FVMatAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("vel", "advection velocity");
  return params;
}

FVMatAdvection::FVMatAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _vel_left(getADMaterialProperty<RealVectorValue>("vel")),
    _vel_right(getNeighborADMaterialProperty<RealVectorValue>("vel"))
{
}

ADReal
FVMatAdvection::computeQpResidual()
{
  auto v_avg = (_vel_left[_qp] + _vel_right[_qp]) * 0.5;
  ADReal r = 0;
  if (v_avg * _normal > 0)
    r = _normal * v_avg * _u_left[_qp];
  else
    r = _normal * v_avg * _u_right[_qp];
  return r;
}
