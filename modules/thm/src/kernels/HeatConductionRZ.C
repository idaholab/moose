#include "HeatConductionRZ.h"

registerMooseObject("THMApp", HeatConductionRZ);

template <>
InputParameters
validParams<HeatConductionRZ>()
{
  InputParameters params = validParams<HeatConductionKernel>();
  params += validParams<RZSymmetry>();
  return params;
}

HeatConductionRZ::HeatConductionRZ(const InputParameters & parameters)
  : HeatConductionKernel(parameters), RZSymmetry(parameters)
{
}

Real
HeatConductionRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpJacobian();
}
