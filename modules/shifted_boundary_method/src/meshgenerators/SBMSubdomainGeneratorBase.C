//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMSubdomainGeneratorBase.h"

InputParameters
SBMSubdomainGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += SBMElementClassificationInterface::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  return params;
}

SBMSubdomainGeneratorBase::SBMSubdomainGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters), SBMElementClassificationInterface(this), _input(getMesh("input"))
{
}
