#include "NodalEnergyFluxPostprocessor.h"

registerMooseObject("THMApp", NodalEnergyFluxPostprocessor);

template <>
InputParameters
validParams<NodalEnergyFluxPostprocessor>()
{
  InputParameters params = validParams<NodalPostprocessor>();
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("H", "Specific total enthalpy");

  return params;
}

NodalEnergyFluxPostprocessor::NodalEnergyFluxPostprocessor(const InputParameters & parameters)
  : NodalPostprocessor(parameters), _arhouA(coupledValue("arhouA")), _H(coupledValue("H"))
{
}

void
NodalEnergyFluxPostprocessor::initialize()
{
  _value = 0;
}

void
NodalEnergyFluxPostprocessor::execute()
{
  _value = _arhouA[_qp] * _H[_qp];
}

PostprocessorValue
NodalEnergyFluxPostprocessor::getValue()
{
  gatherSum(_value);
  return _value;
}

void
NodalEnergyFluxPostprocessor::threadJoin(const UserObject & uo)
{
  const NodalEnergyFluxPostprocessor & niep =
      dynamic_cast<const NodalEnergyFluxPostprocessor &>(uo);
  _value += niep._value;
}
