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
#include "MooseMeshElementConversionUtils.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/boundary_info.h"

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

  // libMesh's MeshTools::Modification::all_tri does not know how to subdivide C0POLYGON or
  // C0POLYHEDRON elements; pre-split those into TRI3 / TET4 using their existing internal
  // triangulations, then hand the remaining (poly-free) mesh to all_tri to handle the standard
  // and higher-order element types it already supports.
  std::vector<dof_id_type> poly_2d, poly_3d;
  for (const Elem * elem : mesh->element_ptr_range())
  {
    if (elem->type() == C0POLYGON)
      poly_2d.push_back(elem->id());
    else if (elem->type() == C0POLYHEDRON)
      poly_3d.push_back(elem->id());
  }

  if (!poly_2d.empty() || !poly_3d.empty())
  {
    auto * replicated_mesh = dynamic_cast<ReplicatedMesh *>(mesh.get());
    if (!replicated_mesh)
      paramError(
          "input",
          "ElementsToSimplicesConverter requires a replicated mesh to subdivide C0POLYGON or "
          "C0POLYHEDRON elements; serialize the distributed mesh first.");

    std::set<subdomain_id_type> subdomain_ids_set;
    mesh->subdomain_ids(subdomain_ids_set);
    const subdomain_id_type max_sid = *subdomain_ids_set.rbegin();
    const subdomain_id_type block_id_to_remove = max_sid + 1;

    BoundaryInfo & boundary_info = mesh->get_boundary_info();
    const auto bdry_side_list = boundary_info.build_side_list();
    std::vector<dof_id_type> converted_elems_ids;
    for (const auto eid : poly_2d)
    {
      MooseMeshElementConversionUtils::polygonElemSplitter(
          *mesh, bdry_side_list, eid, converted_elems_ids);
      mesh->elem_ptr(eid)->subdomain_id() = block_id_to_remove;
    }
    for (const auto eid : poly_3d)
    {
      MooseMeshElementConversionUtils::polyhedronElemSplitter(
          *mesh, bdry_side_list, eid, converted_elems_ids);
      mesh->elem_ptr(eid)->subdomain_id() = block_id_to_remove;
    }

    for (auto elem_it = mesh->active_subdomain_elements_begin(block_id_to_remove);
         elem_it != mesh->active_subdomain_elements_end(block_id_to_remove);
         elem_it++)
      mesh->delete_elem(*elem_it);
    mesh->contract();
    mesh->prepare_for_use();
  }

  MeshTools::Modification::all_tri(*mesh);

  return mesh;
}
