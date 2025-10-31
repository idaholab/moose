//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestOneToManyDependencyMeshGenerator.h"
#include "MeshGenerator.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestOneToManyDependencyMeshGenerator);

InputParameters
TestOneToManyDependencyMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // input parameter that is an existing mesh generator
  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  // additional params for this specific mesh generator
  params.addRequiredParam<unsigned int>(
      "copy_id", "ID given to specify which copy number this MeshGenerator is.");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestOneToManyDependencyMeshGenerator::TestOneToManyDependencyMeshGenerator(
    const InputParameters & params)
  : MeshGenerator(params), _mesh_ptr(getMesh("input")), _copy_id(getParam<unsigned int>("copy_id"))
{
  _build_csg = &getCSGBase("input");
}

std::unique_ptr<MeshBase>
TestOneToManyDependencyMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestOneToManyDependencyMeshGenerator::generateCSG()
{
  // get the existing CSGBase associated with the input mesh generator
  // this is the CSGBase object that will be updated
  std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*_build_csg);

  // Get all surfaces in base object and rename them
  for (const auto & surf : csg_obj->getAllSurfaces())
  {
    const auto surf_name = surf.get().getName();
    const auto new_surf_name = surf_name + "_copy_" + std::to_string(_copy_id);
    csg_obj->renameSurface(surf, new_surf_name);
  }

  // Get all cells in base object and rename them
  for (const auto & cell : csg_obj->getAllCells())
  {
    const auto cell_name = cell.get().getName();
    const auto new_cell_name = cell_name + "_copy_" + std::to_string(_copy_id);
    csg_obj->renameCell(cell, new_cell_name);
  }

  // Get all non-root universes in base object and rename them
  for (const auto & univ : csg_obj->getAllUniverses())
    if (!univ.get().isRoot())
    {
      const auto univ_name = univ.get().getName();
      const auto new_univ_name = univ_name + "_copy_" + std::to_string(_copy_id);
      csg_obj->renameUniverse(univ, new_univ_name);
    }

  return csg_obj;
}
