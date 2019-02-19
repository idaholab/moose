#include "MomentumFluxIntegral.h"

registerMooseObject("THMApp", MomentumFluxIntegral);

template <>
InputParameters
validParams<MomentumFluxIntegral>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addRequiredCoupledVar("arhouA", "Momentum equation variable");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("A", "Area");
  params.addCoupledVar("alpha", 1.0, "Volume fraction (two-phase only)");
  return params;
}

MomentumFluxIntegral::MomentumFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _arhouA(coupledValue("arhouA")),
    _velocity(coupledValue("vel")),
    _pressure(coupledValue("p")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

void
MomentumFluxIntegral::threadJoin(const UserObject & y)
{
  const MomentumFluxIntegral & pps = static_cast<const MomentumFluxIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
MomentumFluxIntegral::computeQpIntegral()
{
  return _arhouA[_qp] * _velocity[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp];
}
