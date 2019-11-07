//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatSideUserObject.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MatSideUserObject);

InputParameters
MatSideUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop", "the name of the material property we are going to use");
  return params;
}

MatSideUserObject::MatSideUserObject(const InputParameters & parameters)
  : SideUserObject(parameters), _mat_prop(getMaterialProperty<Real>("mat_prop"))
{
}
