//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyMaterial.h"

registerMooseObject("RichardsApp", DarcyMaterial);

InputParameters
DarcyMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<RealTensorValue>("mat_permeability",
                                           "The permeability tensor (usually in m^2).");
  params.addClassDescription("Material that holds the permeability tensor used in Darcy flow");
  return params;
}

DarcyMaterial::DarcyMaterial(const InputParameters & parameters)
  : Material(parameters),
    _material_perm(getParam<RealTensorValue>("mat_permeability")),
    _permeability(declareProperty<RealTensorValue>("permeability"))
{
}

void
DarcyMaterial::computeQpProperties()
{
  _permeability[_qp] = _material_perm;
}
