#include "HeatConductionTimeDerivativeRZ.h"

registerMooseObject("THMApp", HeatConductionTimeDerivativeRZ);

template <>
InputParameters
validParams<HeatConductionTimeDerivativeRZ>()
{
  InputParameters params = validParams<HeatConductionTimeDerivative>();
  params += validParams<RZSymmetry>();
  return params;
}

HeatConductionTimeDerivativeRZ::HeatConductionTimeDerivativeRZ(const InputParameters & parameters)
  : HeatConductionTimeDerivative(parameters), RZSymmetry(parameters)
{
}

Real
HeatConductionTimeDerivativeRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionTimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivativeRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionTimeDerivative::computeQpJacobian();
}
