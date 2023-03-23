//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MappingBase.h"

InputParameters
MappingBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += RestartableModelInterface::validParams();
  params.addClassDescription("Base class for mapping objects.");
  params.registerBase("MappingBase");
  params.registerSystemAttributeName("MappingBase");
  return params;
}

MappingBase::MappingBase(const InputParameters & parameters)
  : MooseObject(parameters), RestartableModelInterface(this, _type + "_" + name())
{
}
