#include "HeatStructureEnergyRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureEnergyRZ);

InputParameters
HeatStructureEnergyRZ::validParams()
{
  InputParameters params = HeatStructureEnergyBase::validParams();
  params += RZSymmetry::validParams();
  params.addClassDescription("Computes the total energy for a cylindrical heat structure.");
  return params;
}

HeatStructureEnergyRZ::HeatStructureEnergyRZ(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatStructureEnergyRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureEnergyBase::computeQpIntegral();
}
