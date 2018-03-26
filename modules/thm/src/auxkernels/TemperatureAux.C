#include "TemperatureAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("RELAP7App", TemperatureAux);

template <>
InputParameters
validParams<TemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");

  return params;
}

TemperatureAux::TemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
TemperatureAux::computeValue()
{
  return _fp.T_from_v_e(_v[_qp], _e[_qp]);
}
