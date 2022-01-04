#include "HeatConductionTimeDerivativeRZ.h"

registerMooseObject("THMApp", HeatConductionTimeDerivativeRZ);

InputParameters
HeatConductionTimeDerivativeRZ::validParams()
{
  InputParameters params = HeatConductionTimeDerivative::validParams();
  params += RZSymmetry::validParams();
  return params;
}

HeatConductionTimeDerivativeRZ::HeatConductionTimeDerivativeRZ(const InputParameters & parameters)
  : HeatConductionTimeDerivative(parameters), RZSymmetry(this, parameters)
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
