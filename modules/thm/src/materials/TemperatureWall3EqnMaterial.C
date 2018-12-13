#include "TemperatureWall3EqnMaterial.h"

registerMooseObject("RELAP7App", TemperatureWall3EqnMaterial);

template <>
InputParameters
validParams<TemperatureWall3EqnMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Heat transfer coefficient");
  params.addRequiredCoupledVar("q_wall", "Wall heat flux");

  return params;
}

TemperatureWall3EqnMaterial::TemperatureWall3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),
    _T_wall(declareProperty<Real>("T_wall")),
    _q_wall(coupledValue("q_wall")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _T(getMaterialProperty<Real>("T"))
{
}

void
TemperatureWall3EqnMaterial::computeQpProperties()
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
