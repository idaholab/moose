#include "ADHeatConductionRZ.h"

registerMooseObject("THMApp", ADHeatConductionRZ);

InputParameters
ADHeatConductionRZ::validParams()
{
  InputParameters params = ADHeatConduction::validParams();
  params += RZSymmetry::validParams();
  return params;
}

ADHeatConductionRZ::ADHeatConductionRZ(const InputParameters & parameters)
  : ADHeatConduction(parameters), RZSymmetry(parameters)
{
}

ADRealVectorValue
ADHeatConductionRZ::precomputeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatConduction::precomputeQpResidual();
}
