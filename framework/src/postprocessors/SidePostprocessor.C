//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SidePostprocessor.h"

template <>
InputParameters
validParams<SidePostprocessor>()
{
  InputParameters params = validParams<SideUserObject>();
  params += validParams<Postprocessor>();
  return params;
}

SidePostprocessor::SidePostprocessor(const InputParameters & parameters)
  : SideUserObject(parameters), Postprocessor(parameters)
{
}
