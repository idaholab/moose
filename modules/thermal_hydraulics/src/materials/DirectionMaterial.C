//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirectionMaterial.h"

registerMooseObject("ThermalHydraulicsApp", DirectionMaterial);

InputParameters
DirectionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.set<MooseEnum>("constant_on") = 1; // constant on element
  params.addClassDescription("Computes the direction of 1D elements");
  return params;
}

DirectionMaterial::DirectionMaterial(const InputParameters & parameters)
  : Material(parameters), _dir(declareProperty<RealVectorValue>("direction"))
{
}

void
DirectionMaterial::computeQpProperties()
{
  const Elem * el = _mesh.elemPtr(_current_elem->id());
  RealVectorValue dir = el->node_ref(1) - el->node_ref(0);
  _dir[_qp] = dir.unit();
}
