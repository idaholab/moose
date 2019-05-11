#include "OneDHeatForcingFunctionRZ.h"

registerMooseObject("THMApp", OneDHeatForcingFunctionRZ);

template <>
InputParameters
validParams<OneDHeatForcingFunctionRZ>()
{
  InputParameters params = validParams<OneDHeatForcingFunction>();
  params += validParams<RZSymmetry>();
  return params;
}

OneDHeatForcingFunctionRZ::OneDHeatForcingFunctionRZ(const InputParameters & parameters)
  : OneDHeatForcingFunction(parameters), RZSymmetry(parameters)
{
}

Real
OneDHeatForcingFunctionRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * OneDHeatForcingFunction::computeQpResidual();
}

Real
OneDHeatForcingFunctionRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * OneDHeatForcingFunction::computeQpJacobian();
}
