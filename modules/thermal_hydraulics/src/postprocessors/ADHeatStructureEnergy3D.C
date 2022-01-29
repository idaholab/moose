#include "ADHeatStructureEnergy3D.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatStructureEnergy3D);

InputParameters
ADHeatStructureEnergy3D::validParams()
{
  InputParameters params = ADHeatStructureEnergyBase::validParams();
  params.suppressParameter<Real>("n_units");
  params.addClassDescription("Computes the total energy for a 3D heat structure.");
  return params;
}

ADHeatStructureEnergy3D::ADHeatStructureEnergy3D(const InputParameters & parameters)
  : ADHeatStructureEnergyBase(parameters)
{
}

Real
ADHeatStructureEnergy3D::computeQpIntegral()
{
  return ADHeatStructureEnergyBase::computeQpIntegral();
}
