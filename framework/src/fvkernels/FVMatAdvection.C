
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
    _vel_elem(getADMaterialProperty<RealVectorValue>("vel")),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>("vel"))
{
}

ADReal
FVMatAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal u_interface;
  interpolate(InterpMethod::Average, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  interpolate(InterpMethod::Upwind, u_interface, _u_elem[_qp], _u_neighbor[_qp]);
  return _normal * v * u_interface;
}
