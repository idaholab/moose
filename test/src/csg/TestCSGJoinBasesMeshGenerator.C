//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGJoinBasesMeshGenerator.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGJoinBasesMeshGenerator);

InputParameters
TestCSGJoinBasesMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("input_meshes",
                                                          "list of MGs to add to universe");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGJoinBasesMeshGenerator::TestCSGJoinBasesMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptrs(getMeshes("input_meshes")),
    _input_mgs(getParam<std::vector<MeshGeneratorName>>("input_meshes"))
{
}

std::unique_ptr<MeshBase>
TestCSGJoinBasesMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGJoinBasesMeshGenerator::generateCSG()
{
  // first MG is the base, join all others to this one
  std::unique_ptr<CSG::CSGBase> csg_mesh = std::move(getCSGMeshByName(_input_mgs[0]));

  // rename first MG's root to demonstrate that all others are still joined into
  // root despite having different names
  auto mg_name = this->name();
  csg_mesh->renameRootUniverse(mg_name + "_root");

  auto root_univ = csg_mesh->getRootUniverse();
  for (auto it = _input_mgs.begin() + 1; it != _input_mgs.end(); ++it)
  {
    std::unique_ptr<CSG::CSGBase> inp_csg_mesh = std::move(getCSGMeshByName(*it));
    csg_mesh->joinOtherBase(inp_csg_mesh);
  }

  return csg_mesh;
}