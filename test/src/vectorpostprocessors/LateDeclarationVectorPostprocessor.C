//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LateDeclarationVectorPostprocessor.h"

registerMooseObject("MooseTestApp", LateDeclarationVectorPostprocessor);

InputParameters
LateDeclarationVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<VectorPostprocessorValue>("value",
                                                    "The vector value this object will have.");
  return params;
}

LateDeclarationVectorPostprocessor::LateDeclarationVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _value(nullptr)
{
}

void
LateDeclarationVectorPostprocessor::initialize()
{
  if (_t_step == 1)
    _value = &declareVector("value");
}

void
LateDeclarationVectorPostprocessor::execute()
{
  if (_t_step == 1)
    *_value = getParam<VectorPostprocessorValue>("value");
}
