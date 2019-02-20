#include "MassFluxIntegral.h"

registerMooseObject("THMApp", MassFluxIntegral);

template <>
InputParameters
validParams<MassFluxIntegral>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addRequiredCoupledVar("arhouA", "Momentum equation variable");
  return params;
}

MassFluxIntegral::MassFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters), _arhouA(coupledValue("arhouA"))
{
}

void
MassFluxIntegral::threadJoin(const UserObject & y)
{
  const MassFluxIntegral & pps = static_cast<const MassFluxIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
MassFluxIntegral::computeQpIntegral()
{
  return _arhouA[_qp];
}
