//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideVectorPostprocessor.h"

template <>
InputParameters
validParams<SideVectorPostprocessor>()
{
  InputParameters params = validParams<SideUserObject>();
  params += validParams<VectorPostprocessor>();
  return params;
}

SideVectorPostprocessor::SideVectorPostprocessor(const InputParameters & parameters)
  : SideUserObject(parameters), VectorPostprocessor(parameters)
{
}
