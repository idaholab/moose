//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelBase.h"

InputParameters
GapFluxModelBase::validParams()
{
  InputParameters params = InterfaceUserObject::validParams();
  params.addClassDescription("Gap flux model base class");
  return params;
}

GapFluxModelBase::GapFluxModelBase(const InputParameters & parameters)
  : InterfaceUserObject(parameters)
{
}
