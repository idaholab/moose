//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementVectorPostprocessor.h"

InputParameters
ElementVectorPostprocessor::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params += VectorPostprocessor::validParams();
  return params;
}

ElementVectorPostprocessor::ElementVectorPostprocessor(const InputParameters & parameters)
  : ElementUserObject(parameters), VectorPostprocessor(this)
{
}
