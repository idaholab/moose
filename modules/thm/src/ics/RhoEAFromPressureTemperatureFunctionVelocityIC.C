#include "RhoEAFromPressureTemperatureFunctionVelocityIC.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("THMApp", RhoEAFromPressureTemperatureFunctionVelocityIC);

InputParameters
RhoEAFromPressureTemperatureFunctionVelocityIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");
  params.addRequiredCoupledVar("p", "The pressure");
  params.addRequiredCoupledVar("T", "The temperature");
  params.addRequiredParam<FunctionName>("vel", "The velocity");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

RhoEAFromPressureTemperatureFunctionVelocityIC::RhoEAFromPressureTemperatureFunctionVelocityIC(
    const InputParameters & parameters)
  : InitialCondition(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _p(coupledValue("p")),
    _T(coupledValue("T")),
    _vel(getFunction("vel")),
    _area(coupledValue("A"))
{
}

Real
RhoEAFromPressureTemperatureFunctionVelocityIC::value(const Point & p)
{
  const Real vel = _vel.value(_t, p);
  const Real rho = _fp.rho_from_p_T(_p[_qp], _T[_qp]);
  const Real e = _fp.e_from_p_rho(_p[_qp], rho);
  return rho * (e + 0.5 * vel * vel) * _area[_qp];
}
