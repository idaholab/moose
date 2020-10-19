//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Stateful.h"

registerMooseObject("MooseTestApp", Stateful);

InputParameters
Stateful::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of porosity material property");

  return params;
}

Stateful::Stateful(const InputParameters & parameters)
  : Material(parameters),
    _property(declareProperty<Real>("property_name")),
    _property_old(getMaterialPropertyOld<Real>("property_name"))
{
}

void
Stateful::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
Stateful::computeQpProperties()
{
  _property[_qp] = 0.0;
}
