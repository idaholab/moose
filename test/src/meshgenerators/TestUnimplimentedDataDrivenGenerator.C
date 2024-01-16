//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestUnimplimentedDataDrivenGenerator.h"

registerMooseObject("MooseTestApp", TestUnimplimentedDataDrivenGenerator);

InputParameters
TestUnimplimentedDataDrivenGenerator::validParams()
{
  return MeshGenerator::validParams();
}

TestUnimplimentedDataDrivenGenerator::TestUnimplimentedDataDrivenGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters)
{
}

std::unique_ptr<MeshBase>
TestUnimplimentedDataDrivenGenerator::generate()
{
  generateData();
  return nullptr;
}
