//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlattenRefinementGenerator.h"

#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", FlattenRefinementGenerator);

InputParameters
FlattenRefinementGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Removes the refinement tree of an h-refined mesh, keeping only the finest "
      "elements as unrefined (level-0) elements.");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "The mesh whose refinement will be flattened");
  params.addParam<bool>(
      "first_order",
      false,
      "Whether to also convert the flattened mesh to first-order elements. If false, the element "
      "order of the input mesh is preserved.");
  return params;
}

FlattenRefinementGenerator::FlattenRefinementGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input")), _first_order(getParam<bool>("first_order"))
{
}

std::unique_ptr<MeshBase>
FlattenRefinementGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // flatten() asserts that the mesh is prepared (or replicated) because it relies on boundary
  // information while rebuilding the finest elements as level-0 elements
  if (!mesh->is_prepared())
    mesh->prepare_for_use();

  // Discard the refinement tree, promoting the finest (active) elements to level 0
  MeshTools::Modification::flatten(*mesh);

  // Optionally drop to first order now that the refinement pattern is gone
  if (_first_order)
    mesh->all_first_order();

  // The mesh has been rebuilt, so let the rest of the setup re-prepare it
  mesh->unset_is_prepared();

  return mesh;
}
