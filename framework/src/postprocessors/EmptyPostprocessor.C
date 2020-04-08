//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EmptyPostprocessor.h"

#include "libmesh/parallel.h"

registerMooseObject("MooseApp", EmptyPostprocessor);

defineLegacyParams(EmptyPostprocessor);

InputParameters
EmptyPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

EmptyPostprocessor::EmptyPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
}
