#include "ADHeatConductionTimeDerivativeRZ.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatConductionTimeDerivativeRZ);

InputParameters
ADHeatConductionTimeDerivativeRZ::validParams()
{
  InputParameters params = ADHeatConductionTimeDerivative::validParams();
  params += RZSymmetry::validParams();
  return params;
}

ADHeatConductionTimeDerivativeRZ::ADHeatConductionTimeDerivativeRZ(
    const InputParameters & parameters)
  : ADHeatConductionTimeDerivative(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADHeatConductionTimeDerivativeRZ::precomputeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatConductionTimeDerivative::precomputeQpResidual();
}
