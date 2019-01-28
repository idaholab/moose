#include "RhoEFromPressureTemperatureVelocityIC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", RhoEFromPressureTemperatureVelocityIC);

template <>
InputParameters
validParams<RhoEFromPressureTemperatureVelocityIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");
  params.addRequiredCoupledVar("p", "The pressure");
  params.addRequiredCoupledVar("T", "The temperature");
  params.addRequiredCoupledVar("vel", "The velocity");
  return params;
}

RhoEFromPressureTemperatureVelocityIC::RhoEFromPressureTemperatureVelocityIC(
    const InputParameters & parameters)
  : InitialCondition(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _p(coupledValue("p")),
    _T(coupledValue("T")),
    _vel(coupledValue("vel"))
{
}

Real
RhoEFromPressureTemperatureVelocityIC::value(const Point & /*p*/)
{
  const Real rho = _fp.rho_from_p_T(_p[_qp], _T[_qp]);
  const Real e = _fp.e_from_p_rho(_p[_qp], rho);
  return rho * (e + 0.5 * _vel[_qp] * _vel[_qp]);
}
