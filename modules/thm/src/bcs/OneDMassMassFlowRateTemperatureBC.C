#include "OneDMassMassFlowRateTemperatureBC.h"

registerMooseObject("THMApp", OneDMassMassFlowRateTemperatureBC);

InputParameters
OneDMassMassFlowRateTemperatureBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredParam<Real>("m_dot", "The specified mass flow rate value.");

  params.declareControllable("m_dot");

  return params;
}

OneDMassMassFlowRateTemperatureBC::OneDMassMassFlowRateTemperatureBC(
    const InputParameters & parameters)
  : OneDIntegratedBC(parameters), _m_dot(getParam<Real>("m_dot"))
{
}

Real
OneDMassMassFlowRateTemperatureBC::computeQpResidual()
{
  return _m_dot * _normal * _test[_i][_qp];
}

Real
OneDMassMassFlowRateTemperatureBC::computeQpJacobian()
{
  return 0.;
}
