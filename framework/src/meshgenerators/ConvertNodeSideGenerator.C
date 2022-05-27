//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvertNodeSideGenerator.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_refinement.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", ConvertNodeSideGenerator);

InputParameters
ConvertNodeSideGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which refines one or more blocks in an existing mesh");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to refine");
  params.addRequiredParam<bool>(
      "enable_neighbor_refinement",
      true,
      "Toggles whether neighboring level one elements should be refined or not. Defaults to true");

  return params;
}

ConvertNodeSideGenerator::ConvertNodeSideGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input"))
{
  // do nothing yet
}

std::unique_ptr<MeshBase>
ConvertNodeSideGenerator::generate()
{
  return nullptr;
}
