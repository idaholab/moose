#include "OneDHeatForcingFunctionRZ.h"

registerMooseObject("RELAP7App", OneDHeatForcingFunctionRZ);

template <>
InputParameters
validParams<OneDHeatForcingFunctionRZ>()
{
  InputParameters params = validParams<OneDHeatForcingFunction>();
  return params;
}

OneDHeatForcingFunctionRZ::OneDHeatForcingFunctionRZ(const InputParameters & parameters)
  : OneDHeatForcingFunction(parameters), RZSymmetry(parameters)
{
}

Real
OneDHeatForcingFunctionRZ::computeQpResidual()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2. * libMesh::pi * r * OneDHeatForcingFunction::computeQpResidual();
}

Real
OneDHeatForcingFunctionRZ::computeQpJacobian()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2. * libMesh::pi * r * OneDHeatForcingFunction::computeQpJacobian();
}
