//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementsToSimplicesConverter.h"

#include "CastUniquePointer.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", ElementsToSimplicesConverter);

InputParameters
ElementsToSimplicesConverter::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to convert to all-simplex mesh");

  params.addClassDescription("Splits all non-simplex elements in a mesh into simplices.");

  return params;
}

ElementsToSimplicesConverter::ElementsToSimplicesConverter(const InputParameters & parameters)
  : MeshGenerator(parameters), _input_ptr(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
ElementsToSimplicesConverter::generate()
{
  // Put the input mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input_ptr));

  // all_tri() on a ReplicatedMesh skips its internal prepare_for_use() call (it only prepares
  // for non-replicated meshes). Without preparation, element_ptr_range() may not iterate over
  // all elements correctly, producing incomplete results. Prepare here to ensure correctness.
  if (!mesh->is_prepared())
    mesh->prepare_for_use();

  MeshTools::Modification::all_tri(*mesh);

  return mesh;
}
