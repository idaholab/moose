#include "HeatStructureHeatSourceRZ.h"

registerMooseObject("THMApp", HeatStructureHeatSourceRZ);

InputParameters
HeatStructureHeatSourceRZ::validParams()
{
  InputParameters params = HeatStructureHeatSource::validParams();
  params += RZSymmetry::validParams();
  return params;
}

HeatStructureHeatSourceRZ::HeatStructureHeatSourceRZ(const InputParameters & parameters)
  : HeatStructureHeatSource(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatStructureHeatSourceRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureHeatSource::computeQpResidual();
}

Real
HeatStructureHeatSourceRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureHeatSource::computeQpJacobian();
}
