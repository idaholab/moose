#include "ADHeatStructureHeatSourceRZ.h"

registerMooseObject("THMApp", ADHeatStructureHeatSourceRZ);

InputParameters
ADHeatStructureHeatSourceRZ::validParams()
{
  InputParameters params = ADHeatStructureHeatSource::validParams();
  params += RZSymmetry::validParams();
  return params;
}

ADHeatStructureHeatSourceRZ::ADHeatStructureHeatSourceRZ(const InputParameters & parameters)
  : ADHeatStructureHeatSource(parameters), RZSymmetry(parameters)
{
}

ADReal
ADHeatStructureHeatSourceRZ::computeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatStructureHeatSource::computeQpResidual();
}

Real
ADHeatStructureHeatSourceRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatStructureHeatSource::computeQpJacobian();
}
