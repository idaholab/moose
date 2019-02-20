#include "VolumeJunctionPressureAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", VolumeJunctionPressureAux);

template <>
InputParameters
validParams<VolumeJunctionPressureAux>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addRequiredCoupledVar("junction_rho", "Density form the junction");
  params.addRequiredCoupledVar("junction_rhoe", "Internal energy form the junction");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the fluid properties user object to use.");

  return params;
}

VolumeJunctionPressureAux::VolumeJunctionPressureAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _rho(coupledScalarValue("junction_rho")),
    _rhoe(coupledScalarValue("junction_rhoe")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
VolumeJunctionPressureAux::computeValue()
{
  return _fp.p_from_v_e(1. / _rho[0], _rhoe[0] / _rho[0]);
}
