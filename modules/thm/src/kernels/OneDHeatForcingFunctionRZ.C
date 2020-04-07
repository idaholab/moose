#include "OneDHeatForcingFunctionRZ.h"

registerMooseObject("THMApp", OneDHeatForcingFunctionRZ);

InputParameters
OneDHeatForcingFunctionRZ::validParams()
{
  InputParameters params = OneDHeatForcingFunction::validParams();
  params += RZSymmetry::validParams();
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
