#include "PNSFVMassSpecifiedMassFluxBC.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PNSFVMassSpecifiedMassFluxBC);

InputParameters
PNSFVMassSpecifiedMassFluxBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params.addRequiredParam<FunctionName>(NS::superficial_momentum_x,
                                        "The x component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::superficial_momentum_y,
                                "The y component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::superficial_momentum_z,
                                "The z component of the inlet superficial momentum");
  return params;
}

PNSFVMassSpecifiedMassFluxBC::PNSFVMassSpecifiedMassFluxBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _superficial_rhou(getFunction(NS::superficial_momentum_x)),
    _superficial_rhov(isParamValid(NS::superficial_momentum_y)
                          ? &getFunction(NS::superficial_momentum_y)
                          : nullptr),
    _superficial_rhow(isParamValid(NS::superficial_momentum_z)
                          ? &getFunction(NS::superficial_momentum_z)
                          : nullptr)
{
  if (_mesh.dimension() > 1 && !_superficial_rhov)
    mooseError("If the mesh dimension is greater than 1, a function for the y superficial momentum "
               "must be provided");
  if (_mesh.dimension() > 2 && !_superficial_rhow)
    mooseError("If the mesh dimension is greater than 2, a function for the z superficial momentum "
               "must be provided");
}

void
PNSFVMassSpecifiedMassFluxBC::computeMemberData()
{
  _mass_flux.assign(RealVectorValue(_superficial_rhou.value(_t, _face_info->faceCentroid()), 0, 0));
  if (_superficial_rhov)
    _mass_flux(1) = _superficial_rhov->value(_t, _face_info->faceCentroid());
  if (_superficial_rhow)
    _mass_flux(2) = _superficial_rhow->value(_t, _face_info->faceCentroid());
}

ADReal
PNSFVMassSpecifiedMassFluxBC::computeQpResidual()
{
  computeMemberData();
  return _normal * _mass_flux;
}
