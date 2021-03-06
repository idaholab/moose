#include "NSFVFluidEnergySpecifiedTemperatureBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", NSFVFluidEnergySpecifiedTemperatureBC);

InputParameters
NSFVFluidEnergySpecifiedTemperatureBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params.addRequiredParam<FunctionName>(NS::temperature,
                                        "Inlet temperature specified as a function");
  params.addRequiredParam<FunctionName>(NS::momentum_x,
                                        "The x component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::momentum_y,
                                "The y component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::momentum_z,
                                "The z component of the inlet superficial momentum");
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  return params;
}

NSFVFluidEnergySpecifiedTemperatureBC::NSFVFluidEnergySpecifiedTemperatureBC(
    const InputParameters & parameters)
  : FVFluxBC(parameters),
    _temperature(getFunction(NS::temperature)),
    _rhou(getFunction(NS::momentum_x)),
    _rhov(isParamValid(NS::momentum_y) ? &getFunction(NS::momentum_y) : nullptr),
    _rhow(isParamValid(NS::momentum_z) ? &getFunction(NS::momentum_z) : nullptr),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(NS::density)),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid))
{
  if (_mesh.dimension() > 1 && !_rhov)
    mooseError("If the mesh dimension is greater than 1, a function for the y superficial momentum "
               "must be provided");
  if (_mesh.dimension() > 2 && !_rhow)
    mooseError("If the mesh dimension is greater than 2, a function for the z superficial momentum "
               "must be provided");
}

ADReal
NSFVFluidEnergySpecifiedTemperatureBC::computeQpResidual()
{
  ADReal rho;
  using namespace Moose::FV;
  interpolate(InterpMethod::Average, rho, _rho_elem[_qp], _rho_neighbor[_qp], *_face_info, true);

  const ADReal T = _temperature.value(_t, _face_info->faceCentroid());
  const ADReal v = 1 / rho;
  const auto e = _fluid.e_from_T_v(T, v);

  RealVectorValue mass_flux(_rhou.value(_t, _face_info->faceCentroid()));
  ADRealVectorValue velocity(mass_flux(0) / rho);
  if (_rhov)
  {
    mass_flux(1) = _rhov->value(_t, _face_info->faceCentroid());
    velocity(1) = mass_flux(1) / rho;
  }
  if (_rhow)
  {
    mass_flux(2) = _rhow->value(_t, _face_info->faceCentroid());
    velocity(2) = mass_flux(2) / rho;
  }

  const auto pressure = _fluid.p_from_T_v(T, v);
  const auto ht = e + 0.5 * velocity * velocity + pressure / rho;
  return _normal * mass_flux * ht;
}
