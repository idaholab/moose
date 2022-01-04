#include "EnergyFluxIntegral.h"

registerMooseObject("THMApp", EnergyFluxIntegral);

InputParameters
EnergyFluxIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("H", "Specific total enthalpy");
  return params;
}

EnergyFluxIntegral::EnergyFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _arhouA(coupledValue("arhouA")),
    _enthalpy(coupledValue("H"))
{
}

void
EnergyFluxIntegral::threadJoin(const UserObject & y)
{
  const EnergyFluxIntegral & pps = static_cast<const EnergyFluxIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
EnergyFluxIntegral::computeQpIntegral()
{
  return _arhouA[_qp] * _enthalpy[_qp];
}
