//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryBase.h"

InputParameters
BoundaryBase::validParams()
{
  InputParameters params = Component::validParams();
  params.addPrivateParam<std::string>("component_type", "boundary");
  return params;
}

BoundaryBase::BoundaryBase(const InputParameters & params) : Component(params) {}
