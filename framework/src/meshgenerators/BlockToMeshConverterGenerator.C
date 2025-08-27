//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockToMeshConverterGenerator.h"
#include "CastUniquePointer.h"
#include "libmesh/elem.h"
#include "MooseMeshUtils.h"

registerMooseObject("MooseApp", BlockToMeshConverterGenerator);

InputParameters
BlockToMeshConverterGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Converts one or more blocks (subdomains) from a mesh into a stand-alone mesh with a "
      "single block in it.");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "target_blocks",
      "The (list of) blocks (or 'subdomains') we wish to have moved to a new mesh (by name, not "
      "ID)");

  return params;
}

BlockToMeshConverterGenerator::BlockToMeshConverterGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _target_blocks(getParam<std::vector<SubdomainName>>("target_blocks"))
{
}

std::unique_ptr<MeshBase>
BlockToMeshConverterGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto new_mesh = buildMeshBaseObject();

  try
  {
    MooseMeshUtils::convertBlockToMesh(mesh, new_mesh, _target_blocks);
  }
  catch (MooseException & e)
  {
    paramError("target_blocks", e.what());
  }

  return dynamic_pointer_cast<MeshBase>(new_mesh);
}
