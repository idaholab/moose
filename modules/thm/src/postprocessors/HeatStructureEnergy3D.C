#include "HeatStructureEnergy3D.h"

registerMooseObject("THMApp", HeatStructureEnergy3D);

InputParameters
HeatStructureEnergy3D::validParams()
{
  InputParameters params = HeatStructureEnergyBase::validParams();
  params.suppressParameter<Real>("n_units");
  params.addClassDescription("Computes the total energy for a 3D heat structure.");
  return params;
}

HeatStructureEnergy3D::HeatStructureEnergy3D(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters)
{
}

Real
HeatStructureEnergy3D::computeQpIntegral()
{
  return HeatStructureEnergyBase::computeQpIntegral();
}
