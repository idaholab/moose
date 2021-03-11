#include "PNSFVFluidEnergySpecifiedTemperatureBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PNSFVFluidEnergySpecifiedTemperatureBC);

InputParameters
PNSFVFluidEnergySpecifiedTemperatureBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params.addRequiredParam<FunctionName>(NS::temperature,
                                        "Inlet temperature specified as a function");
  params.addRequiredParam<FunctionName>(NS::superficial_momentum_x,
                                        "The x component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::superficial_momentum_y,
                                "The y component of the inlet superficial momentum");
  params.addParam<FunctionName>(NS::superficial_momentum_z,
                                "The z component of the inlet superficial momentum");
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  MooseEnum interp_method("average upwind", "upwind");
  params.addParam<MooseEnum>("interp_method",
                             interp_method,
                             "The interpolation to use. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  return params;
}

PNSFVFluidEnergySpecifiedTemperatureBC::PNSFVFluidEnergySpecifiedTemperatureBC(
    const InputParameters & parameters)
  : FVFluxBC(parameters),
    _temperature(getFunction(NS::temperature)),
    _superficial_rhou(getFunction(NS::superficial_momentum_x)),
    _superficial_rhov(isParamValid(NS::superficial_momentum_y)
                          ? &getFunction(NS::superficial_momentum_y)
                          : nullptr),
    _superficial_rhow(isParamValid(NS::superficial_momentum_z)
                          ? &getFunction(NS::superficial_momentum_z)
                          : nullptr),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(NS::density)),
    _porosity_elem(getMaterialProperty<Real>(NS::porosity)),
    _porosity_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid))
{
  if (_mesh.dimension() > 1 && !_superficial_rhov)
    mooseError("If the mesh dimension is greater than 1, a function for the y superficial momentum "
               "must be provided");
  if (_mesh.dimension() > 2 && !_superficial_rhow)
    mooseError("If the mesh dimension is greater than 2, a function for the z superficial momentum "
               "must be provided");

  using namespace Moose::FV;

  const auto & interp_method = getParam<MooseEnum>("interp_method");
  if (interp_method == "average")
    _interp_method = InterpMethod::Average;
  else if (interp_method == "upwind")
    _interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ", static_cast<std::string>(interp_method));
}

ADReal
PNSFVFluidEnergySpecifiedTemperatureBC::computeQpResidual()
{
  using namespace Moose::FV;

  mooseAssert(_porosity_elem[_qp] == _porosity_neighbor[_qp],
              "It's not good if a ghost cell porosity has a different porosity than the boundary "
              "cell porosity");
  const auto & porosity = _porosity_elem[_qp];

  RealVectorValue mass_flux(_superficial_rhou.value(_t, _face_info->faceCentroid()));
  if (_superficial_rhov)
    mass_flux(1) = _superficial_rhov->value(_t, _face_info->faceCentroid());
  if (_superficial_rhow)
    mass_flux(2) = _superficial_rhow->value(_t, _face_info->faceCentroid());

  ADReal rho;
  interpolate(
      _interp_method, rho, _rho_elem[_qp], _rho_neighbor[_qp], mass_flux, *_face_info, true);

  const ADReal T = _temperature.value(_t, _face_info->faceCentroid());
  const ADReal v = 1 / rho;
  const auto e = _fluid.e_from_T_v(T, v);

  ADRealVectorValue velocity(mass_flux(0) / rho / porosity);
  if (_superficial_rhov)
    velocity(1) = mass_flux(1) / rho / porosity;
  if (_superficial_rhow)
    velocity(2) = mass_flux(2) / rho / porosity;

  const auto pressure = _fluid.p_from_T_v(T, v);
  const auto ht = e + 0.5 * velocity * velocity + pressure / rho;
  return _normal * mass_flux * ht;
}
