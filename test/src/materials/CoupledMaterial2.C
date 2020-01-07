//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledMaterial2.h"

registerMooseObject("MooseTestApp", CoupledMaterial2);

InputParameters
CoupledMaterial2::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the property this material defines");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop1", "Name of the first property to couple into this material");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop2", "Name of the second property to couple into this material");
  return params;
}

CoupledMaterial2::CoupledMaterial2(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop")),
    _mat_prop(declareProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop(getMaterialProperty<Real>("coupled_mat_prop1")),
    _coupled_mat_prop2(getMaterialProperty<Real>("coupled_mat_prop2"))
{
}

void
CoupledMaterial2::computeQpProperties()
{
  _mat_prop[_qp] = 4.0 / _coupled_mat_prop[_qp] /
                   _coupled_mat_prop2[_qp]; // This will produce a NaN if evaluated out of order
}
