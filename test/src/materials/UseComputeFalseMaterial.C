//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UseComputeFalseMaterial.h"

registerMooseObject("MooseTestApp", UseComputeFalseMaterial);

template <>
InputParameters
validParams<UseComputeFalseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Instructs a ComputeFalseMaterial to use its computeQpThings method");
  params.addRequiredParam<MaterialName>("compute_false_material",
                                        "Name of the ComputeFalseMaterial");
  return params;
}

UseComputeFalseMaterial::UseComputeFalseMaterial(const InputParameters & parameters)
  : Material(parameters),
    _compute_false_material(dynamic_cast<ComputeFalseMaterial &>(
        getMaterialByName(getParam<MaterialName>("compute_false_material"))))
{
}

void
UseComputeFalseMaterial::computeQpProperties()
{
  _compute_false_material.setQp(_qp);
  _compute_false_material.computeQpThings();
}
