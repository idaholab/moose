#include "ADTemperatureWall3EqnMaterial.h"

registerMooseObject("THMApp", ADTemperatureWall3EqnMaterial);

InputParameters
ADTemperatureWall3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Heat transfer coefficient");
  params.addRequiredParam<MaterialPropertyName>("q_wall", "Wall heat flux");

  return params;
}

ADTemperatureWall3EqnMaterial::ADTemperatureWall3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),
    _T_wall(declareADProperty<Real>("T_wall")),
    _q_wall(getADMaterialProperty<Real>("q_wall")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T(getADMaterialProperty<Real>("T"))
{
}

void
ADTemperatureWall3EqnMaterial::computeQpProperties()
{
  if (_q_wall[_qp] == 0)
    _T_wall[_qp] = _T[_qp];
  else
  {
    mooseAssert(_Hw[_qp] != 0,
                "The wall heat transfer coefficient is zero, yet the wall heat flux is nonzero.");
    _T_wall[_qp] = _q_wall[_qp] / _Hw[_qp] + _T[_qp];
  }
}
