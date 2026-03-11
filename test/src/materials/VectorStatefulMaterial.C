//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorStatefulMaterial.h"

registerMooseObject("MooseTestApp", VectorStatefulMaterial);

InputParameters
VectorStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Test material declaring a RealVectorValue stateful property named "
                             "'diffusivity' for type mismatch testing.");
  return params;
}

VectorStatefulMaterial::VectorStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),
    _diffusivity(declareProperty<RealVectorValue>("diffusivity")),
    _diffusivity_old(getMaterialPropertyOld<RealVectorValue>("diffusivity"))
{
}

void
VectorStatefulMaterial::initQpStatefulProperties()
{
  _diffusivity[_qp] = RealVectorValue(0, 0, 0);
}

void
VectorStatefulMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _diffusivity_old[_qp];
}
