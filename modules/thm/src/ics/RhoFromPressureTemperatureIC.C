#include "RhoFromPressureTemperatureIC.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<RhoFromPressureTemperatureIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object.");
  params.addRequiredCoupledVar("p", "The pressure");
  params.addRequiredCoupledVar("T", "The temperature");
  return params;
}

RhoFromPressureTemperatureIC::RhoFromPressureTemperatureIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _spfp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _p(coupledValue("p")),
    _T(coupledValue("T"))
{
}

Real
RhoFromPressureTemperatureIC::value(const Point & /*p*/)
{
  return _spfp.rho(_p[_qp], _T[_qp]);
}
