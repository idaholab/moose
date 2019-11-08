//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledMaterial.h"

registerMooseObject("MooseTestApp", CoupledMaterial);

InputParameters
CoupledMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the property this material defines");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop", "Name of the property to couple into this material");
  params.addParam<bool>(
      "use_old_prop",
      false,
      "Boolean indicating whether to use the old coupled property instead of the current property");
  return params;
}

CoupledMaterial::CoupledMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop")),
    _mat_prop(declareProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop(getParam<bool>("use_old_prop")
                          ? getMaterialPropertyOld<Real>("coupled_mat_prop")
                          : getMaterialProperty<Real>("coupled_mat_prop"))
{
}

void
CoupledMaterial::computeQpProperties()
{
  _mat_prop[_qp] =
      4.0 / _coupled_mat_prop[_qp]; // This will produce a NaN if evaluated out of order
}
