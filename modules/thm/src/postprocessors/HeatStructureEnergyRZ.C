#include "HeatStructureEnergyRZ.h"

registerMooseObject("THMApp", HeatStructureEnergyRZ);

template <>
InputParameters
validParams<HeatStructureEnergyRZ>()
{
  InputParameters params = validParams<HeatStructureEnergyBase>();
  params += validParams<RZSymmetry>();
  params.addClassDescription("Computes the total energy for a cylindrical heat structure.");
  return params;
}

HeatStructureEnergyRZ::HeatStructureEnergyRZ(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters), RZSymmetry(parameters)
{
}

Real
HeatStructureEnergyRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureEnergyBase::computeQpIntegral();
}
