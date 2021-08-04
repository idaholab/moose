//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EmptyVectorPostprocessor.h"

registerMooseObject("MooseTestApp", EmptyVectorPostprocessor);

InputParameters
EmptyVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "This is a VectorPostprocessor that does not compute any values. Used for testing.");
  return params;
}

EmptyVectorPostprocessor::EmptyVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters)
{
}

void
EmptyVectorPostprocessor::initialize()
{
}

void
EmptyVectorPostprocessor::execute()
{
}
