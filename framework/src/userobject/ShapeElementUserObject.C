//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShapeElementUserObject.h"

template <>
InputParameters
validParams<ShapeElementUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params += ShapeUserObject<ElementUserObject>::validParams();
  return params;
}

ShapeElementUserObject::ShapeElementUserObject(const InputParameters & parameters)
  : ShapeUserObject<ElementUserObject>(parameters, ShapeType::Element)
{
}
