//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemperatureWall3EqnMaterial.h"

registerMooseObject("ThermalHydraulicsApp", TemperatureWall3EqnMaterial);

InputParameters
TemperatureWall3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Heat transfer coefficient");
  params.addRequiredParam<MaterialPropertyName>("q_wall", "Wall heat flux");

  return params;
}

TemperatureWall3EqnMaterial::TemperatureWall3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),
    _T_wall(declareProperty<Real>("T_wall")),
    _q_wall(getMaterialProperty<Real>("q_wall")),
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
