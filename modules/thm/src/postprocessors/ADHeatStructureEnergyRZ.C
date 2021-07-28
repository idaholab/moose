#include "ADHeatStructureEnergyRZ.h"

registerMooseObject("THMApp", ADHeatStructureEnergyRZ);

InputParameters
ADHeatStructureEnergyRZ::validParams()
{
  InputParameters params = ADHeatStructureEnergyBase::validParams();
  params += RZSymmetry::validParams();
  params.addClassDescription("Computes the total energy for a cylindrical heat structure.");
  return params;
}

ADHeatStructureEnergyRZ::ADHeatStructureEnergyRZ(const InputParameters & parameters)
  : ADHeatStructureEnergyBase(parameters), RZSymmetry(parameters)
{
}

Real
ADHeatStructureEnergyRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatStructureEnergyBase::computeQpIntegral();
}
