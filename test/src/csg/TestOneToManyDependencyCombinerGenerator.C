//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestOneToManyDependencyCombinerGenerator.h"
#include "MeshGenerator.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestOneToManyDependencyCombinerGenerator);

InputParameters
TestOneToManyDependencyCombinerGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // input parameter that is an existing mesh generator
  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestOneToManyDependencyCombinerGenerator::TestOneToManyDependencyCombinerGenerator(
    const InputParameters & params)
  : MeshGenerator(params), _mesh_ptr(getMeshes("inputs"))
{
  _build_csg_bases = getCSGBases("inputs");
}

std::unique_ptr<MeshBase>
TestOneToManyDependencyCombinerGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestOneToManyDependencyCombinerGenerator::generateCSG()
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // Combine all bases from inputs into this base
  for (const auto & csg_base : _build_csg_bases)
    csg_obj->joinOtherBase(std::move(*csg_base));

  return csg_obj;
}
