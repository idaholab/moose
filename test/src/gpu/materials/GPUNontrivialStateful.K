//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUNontrivialStateful.h"

registerMooseObject("MooseApp", GPUNontrivialStateful);

// The purpose of this material is to confirm that the load and store functions for GPU material
// properties are only instantiated when they were requested as stateful, so that it doesn't error
// when a user uses a non-trivially-copyable type as a GPU material property without writing custom
// dataStore but never requests it as stateful

InputParameters
GPUNontrivialStateful::validParams()
{
  InputParameters params = GPUMaterial::validParams();
  return params;
}

GPUNontrivialStateful::GPUNontrivialStateful(const InputParameters & parameters)
  : GPUMaterial(parameters)
{
  _prop = declareGPUProperty<NotTriviallyCopyable>("prop");
  // Uncommenting the following line will lead to a compile error that custom dataStore should be
  // provided
  // _old_prop = getGPUMaterialPropertyOld<NotTriviallyCopyable>("prop");
}
