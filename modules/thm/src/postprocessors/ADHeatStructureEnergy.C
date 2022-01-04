#include "ADHeatStructureEnergy.h"

registerMooseObject("THMApp", ADHeatStructureEnergy);

InputParameters
ADHeatStructureEnergy::validParams()
{
  InputParameters params = ADHeatStructureEnergyBase::validParams();

  params.addClassDescription("Computes the total energy for a plate heat structure.");

  params.addRequiredParam<Real>("plate_depth", "Depth of the heat structure if plate-type");

  return params;
}

ADHeatStructureEnergy::ADHeatStructureEnergy(const InputParameters & parameters)
  : ADHeatStructureEnergyBase(parameters), _plate_depth(getParam<Real>("plate_depth"))
{
  addMooseVariableDependency(_T_var);
}

Real
ADHeatStructureEnergy::computeQpIntegral()
{
  return ADHeatStructureEnergyBase::computeQpIntegral() * _plate_depth;
}
