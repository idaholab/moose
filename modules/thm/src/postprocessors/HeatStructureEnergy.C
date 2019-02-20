#include "HeatStructureEnergy.h"

registerMooseObject("THMApp", HeatStructureEnergy);

template <>
InputParameters
validParams<HeatStructureEnergy>()
{
  InputParameters params = validParams<HeatStructureEnergyBase>();

  params.addClassDescription("Computes the total energy for a plate heat structure.");

  params.addRequiredParam<Real>("plate_depth", "Depth of the heat structure if plate-type");

  return params;
}

HeatStructureEnergy::HeatStructureEnergy(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters), _plate_depth(getParam<Real>("plate_depth"))
{
  addMooseVariableDependency(_T_var);
}

Real
HeatStructureEnergy::computeQpIntegral()
{
  return HeatStructureEnergyBase::computeQpIntegral() * _plate_depth;
}
