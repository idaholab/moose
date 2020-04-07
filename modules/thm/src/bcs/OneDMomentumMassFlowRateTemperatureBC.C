#include "OneDMomentumMassFlowRateTemperatureBC.h"

registerMooseObject("THMApp", OneDMomentumMassFlowRateTemperatureBC);

InputParameters
OneDMomentumMassFlowRateTemperatureBC::validParams()
{
  InputParameters params = OneDNodalBC::validParams();
  params.addRequiredParam<Real>("m_dot", "The specified mass flow rate value.");

  params.declareControllable("m_dot");

  return params;
}

OneDMomentumMassFlowRateTemperatureBC::OneDMomentumMassFlowRateTemperatureBC(
    const InputParameters & parameters)
  : OneDNodalBC(parameters), _m_dot(getParam<Real>("m_dot"))
{
}

Real
OneDMomentumMassFlowRateTemperatureBC::computeQpResidual()
{
  return _u[_qp] - _m_dot;
}

Real
OneDMomentumMassFlowRateTemperatureBC::computeQpJacobian()
{
  return 1;
}
