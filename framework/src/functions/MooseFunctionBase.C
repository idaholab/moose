//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseFunctionBase.h"

InputParameters
MooseFunctionBase::validParams()
{
  auto params = MooseObject::validParams();
  params += SetupInterface::validParams();
  return params;
}

MooseFunctionBase::MooseFunctionBase(const InputParameters & params)
  : MooseObject(params), SetupInterface(this)
{
}
