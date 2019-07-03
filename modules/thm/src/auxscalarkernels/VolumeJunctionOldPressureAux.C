#include "VolumeJunctionOldPressureAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", VolumeJunctionOldPressureAux);

template <>
InputParameters
validParams<VolumeJunctionOldPressureAux>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addRequiredCoupledVar("junction_rho", "Density form the junction");
  params.addRequiredCoupledVar("junction_rhoe", "Internal energy form the junction");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the fluid properties user object to use.");

  return params;
}

VolumeJunctionOldPressureAux::VolumeJunctionOldPressureAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _rho(coupledScalarValue("junction_rho")),
    _rhoe(coupledScalarValue("junction_rhoe")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
VolumeJunctionOldPressureAux::computeValue()
{
  return _fp.p_from_v_e(1. / _rho[0], _rhoe[0] / _rho[0]);
}
