#include "ADHeatFluxFromHeatStructure3EqnUserObject.h"

registerMooseObject("THMApp", ADHeatFluxFromHeatStructure3EqnUserObject);

InputParameters
ADHeatFluxFromHeatStructure3EqnUserObject::validParams()
{
  InputParameters params = ADHeatFluxFromHeatStructureBaseUserObject::validParams();
  params.addRequiredParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Convective heat transfer coefficient");
  params.addClassDescription(
      "Cache the heat flux between a single phase flow channel and a heat structure");
  return params;
}

ADHeatFluxFromHeatStructure3EqnUserObject::ADHeatFluxFromHeatStructure3EqnUserObject(
    const InputParameters & parameters)
  : ADHeatFluxFromHeatStructureBaseUserObject(parameters),
    _T_wall(getADMaterialProperty<Real>("T_wall")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T(getADMaterialProperty<Real>("T"))
{
}

ADReal
ADHeatFluxFromHeatStructure3EqnUserObject::computeQpHeatFlux()
{
  return _Hw[_qp] * (_T_wall[_qp] - _T[_qp]);
}
