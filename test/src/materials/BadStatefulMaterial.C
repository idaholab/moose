//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BadStatefulMaterial.h"

registerMooseObject("MooseTestApp", BadStatefulMaterial);

InputParameters
BadStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<bool>("get_older", false, "true to retrieve older property instead of old");
  return params;
}

BadStatefulMaterial::BadStatefulMaterial(const InputParameters & parameters) : Material(parameters)
{
  if (getParam<bool>("get_older"))
    getMaterialPropertyOlder<Real>("nonexistingpropertyname");
  else
    getMaterialPropertyOld<Real>("nonexistingpropertyname");
}

void
BadStatefulMaterial::initQpStatefulProperties()
{
}

void
BadStatefulMaterial::computeQpProperties()
{
}
