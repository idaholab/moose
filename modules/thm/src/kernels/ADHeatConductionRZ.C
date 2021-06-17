#include "ADHeatConductionRZ.h"

registerMooseObject("THMApp", ADHeatConductionRZ);

InputParameters
ADHeatConductionRZ::validParams()
{
  InputParameters params = HeatConductionKernel::validParams();
  params += RZSymmetry::validParams();
  return params;
}

ADHeatConductionRZ::ADHeatConductionRZ(const InputParameters & parameters)
  : HeatConductionKernel(parameters), RZSymmetry(parameters)
{
}

ADReal
ADHeatConductionRZ::precomputeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpResidual();
}
