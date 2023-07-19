//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LikelihoodFunctionBase.h"

InputParameters
LikelihoodFunctionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription("Base class for likelihood functions");
  params.registerBase("LikelihoodFunctionBase");
  params.registerSystemAttributeName("LikelihoodFunctionBase");
  return params;
}

LikelihoodFunctionBase::LikelihoodFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters)
{
}
