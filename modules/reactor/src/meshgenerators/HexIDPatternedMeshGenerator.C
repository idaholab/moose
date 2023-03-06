//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexIDPatternedMeshGenerator.h"
#include "ReportingIDGeneratorUtils.h"

registerMooseObject("ReactorApp", HexIDPatternedMeshGenerator);

InputParameters
HexIDPatternedMeshGenerator::validParams()
{
  InputParameters params = PatternedHexMeshGenerator::validParams();
  return params;
}

HexIDPatternedMeshGenerator::HexIDPatternedMeshGenerator(const InputParameters & parameters)
  : PatternedHexMeshGenerator(parameters)
{
  mooseDeprecated("Please use PatternedHexMeshGenerator instead. The reporting ID capabilities "
                  "were moved to PatternedHexMeshGenerator.");
}

std::unique_ptr<MeshBase>
HexIDPatternedMeshGenerator::generate()
{
  auto mesh = PatternedHexMeshGenerator::generate();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
