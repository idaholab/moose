#include "PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC);

InputParameters
PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC::validParams()
{
  auto params = PCNSFVMomentumAdvectionSpecifiedMassFluxBC::validParams();
  params.addRequiredParam<FunctionName>(NS::temperature, "temperature specified as a function");
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  return params;
}

PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC::
    PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC(const InputParameters & parameters)
  : PCNSFVMomentumAdvectionSpecifiedMassFluxBC(parameters),
    _temperature(getFunction(NS::temperature)),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid))
{
}

ADReal
PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC::computeQpResidual()
{
  // mooseAssert(_var.hasBlocks(_face_info->elem().subdomain_id()), "Checking block restriction");

  computeMemberData();

  const ADReal T = _temperature.value(_t, _face_info->faceCentroid());
  const ADReal v = 1 / _rho[_qp];
  const auto e = _fluid.e_from_T_v(T, v);

  const auto pressure = _fluid.p_from_T_v(T, v);
  const auto ht = e + 0.5 * _velocity * _velocity + pressure / _rho[_qp];
  return _normal * _mass_flux * ht;
}
