//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MergedMeshGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", MergedMeshGenerator);

template <>
InputParameters
validParams<MergedMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();
  params.addClassDescription("Merge multiple meshes into a single unconnected mesh.");
  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  return params;
}

MergedMeshGenerator::MergedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input_names(getParam<std::vector<MeshGeneratorName>>("inputs"))
{
  // error check
  if (_input_names.empty())
    paramError("input_names", "You need to specify at least one MeshGenerator as an input.");

  // Grab the input mesh references as pointers
  for (auto & input_name : _input_names)
    _meshes.push_back(&getMeshByName(input_name));
}

std::unique_ptr<MeshBase>
MergedMeshGenerator::generate()
{
  // merge all meshes into the first one
  auto mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[0]);
  if (!mesh)
    paramError("inputs", _input_names[0], " is not a valid unstructured mesh");

  // Read in all of the other meshes
  for (MooseIndex(_meshes) i = 1; i < _meshes.size(); ++i)
  {
    auto other_mesh = dynamic_cast<UnstructuredMesh *>(_meshes[i]->get());
    if (!other_mesh)
      paramError("inputs", _input_names[i], " is not a valid unstructured mesh");

    dof_id_type node_delta = mesh->max_node_id();
    dof_id_type elem_delta = mesh->max_elem_id();

    unique_id_type unique_delta =
#ifdef LIBMESH_ENABLE_UNIQUE_ID
        mesh->parallel_max_unique_id();
#else
        0;
#endif

    // Copy mesh data over from the other mesh
    mesh->copy_nodes_and_elements(*other_mesh,
                                  /*skip_find_neighbors = */ false,
                                  elem_delta,
                                  node_delta,
                                  unique_delta);
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
