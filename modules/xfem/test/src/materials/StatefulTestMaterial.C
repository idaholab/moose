/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StatefulTestMaterial.h"

template <>
InputParameters
validParams<StatefulTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("");
  return params;
}

StatefulTestMaterial::StatefulTestMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop(declareProperty<Real>("prop3")),
    _prop_old(getMaterialPropertyOld<Real>("prop3")),
    _prop1_old(getMaterialPropertyOld<Real>("prop1"))
{
}

void
StatefulTestMaterial::resetQpProperties()
{
  _prop[_qp] = 0.12345;
}

void
StatefulTestMaterial::computeQpProperties()
{
  Real old_value = _prop_old[_qp];
  _prop[_qp] = old_value + _dt;
}

void
StatefulTestMaterial::initQpStatefulProperties()
{
  _prop[_qp] = _q_point[_qp](0);
}
