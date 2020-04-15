
#include "FVAdvection.h"

registerADMooseObject("MooseApp", FVAdvection);

InputParameters
FVAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

FVAdvection::FVAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _velocity(getParam<RealVectorValue>("velocity"))
{};

ADReal
FVAdvection::computeQpResidual()
{
  ADReal r = 0;
  if (_velocity * _normal > 0)
    r = _normal * _velocity * _u_left[_qp];
  else
    r = _normal * _velocity * _u_right[_qp];
  return r;
}
