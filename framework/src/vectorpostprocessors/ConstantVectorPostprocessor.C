//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantVectorPostprocessor.h"

template <>
InputParameters
validParams<ConstantVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<VectorPostprocessorValue>("value",
                                                    "The vector value this object will have.");
  return params;
}

ConstantVectorPostprocessor::ConstantVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _value(declareVector("value"))
{
  _value = getParam<VectorPostprocessorValue>("value");
}

void
ConstantVectorPostprocessor::initialize()
{
}

void
ConstantVectorPostprocessor::execute()
{
}
