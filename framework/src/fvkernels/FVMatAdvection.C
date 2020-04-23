
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
  ADRealVectorValue v;
  ADReal u_interface;
  interpolate(InterpMethod::Average, v, _vel_left[_qp], _vel_right[_qp]);
  interpolate(InterpMethod::Upwind, u_interface, _u_left[_qp], _u_right[_qp]);
  return _normal * v * u_interface;
}
