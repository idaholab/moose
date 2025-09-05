//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGUnlinkedUniverseMeshGenerator.h"
#include "MeshGenerator.h"

registerMooseObject("MooseTestApp", TestCSGUnlinkedUniverseMeshGenerator);

InputParameters
TestCSGUnlinkedUniverseMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs", "list of MGs providing CSG to add to first input's CSG");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGUnlinkedUniverseMeshGenerator::TestCSGUnlinkedUniverseMeshGenerator(
    const InputParameters & params)
  : MeshGenerator(params),
    _mesh_ptrs(getMeshes("inputs")),
    _input_mgs(getParam<std::vector<MeshGeneratorName>>("inputs"))
{
}

std::unique_ptr<MeshBase>
TestCSGUnlinkedUniverseMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGUnlinkedUniverseMeshGenerator::generateCSG()
{
  auto mg_name = this->name();
  std::unique_ptr<CSG::CSGBase> csg_obj = std::move(getCSGBaseByName(_input_mgs[0]));

  // for all input meshes from the second one onwards, join CSGBase as a universe without linking
  // to root universe of first one
  for (unsigned int i = 1; i < _input_mgs.size(); ++i)
  {
    auto img = _input_mgs[i];
    auto new_join_name = img + "_univ";
    csg_obj->joinOtherBase(std::move(getCSGBaseByName(img)), new_join_name);
  }

  return csg_obj;
}
