//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "LibmeshMFEMMesh.h"
#include "libmesh/int_range.h"

#include <algorithm>
#include <set>

LibmeshMFEMMesh::LibmeshMFEMMesh(
    const int num_elements_in_mesh,
    const LibmeshMFEMBlockInfo & block_info,
    const std::vector<subdomain_id_type> & unique_block_ids,
    const std::map<subdomain_id_type, std::string> & block_ids_to_names,
    const std::vector<boundary_id_type> & unique_side_boundary_ids,
    const std::map<boundary_id_type, std::string> & bound_ids_to_names,
    const std::vector<dof_id_type> & unique_libmesh_corner_node_ids,
    const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
    const NodeIDsForElementID & libmesh_node_ids_for_element_id,
    const NodeIDsForBoundaryID & libmesh_node_ids_for_boundary_id,
    const SideIDsForBoundaryID & libmesh_side_ids_for_boundary_id,
    const BlockIDsForBoundaryID & libmesh_block_ids_for_boundary_id,
    const CoordinatesForNodeID & coordinates_for_libmesh_node_id)
{
  buildMFEMVerticesAndElements(num_elements_in_mesh,
                               block_info,
                               unique_block_ids,
                               block_ids_to_names,
                               unique_side_boundary_ids,
                               bound_ids_to_names,
                               unique_libmesh_corner_node_ids,
                               libmesh_element_ids_for_block_id,
                               libmesh_node_ids_for_element_id,
                               libmesh_node_ids_for_boundary_id,
                               libmesh_side_ids_for_boundary_id,
                               libmesh_block_ids_for_boundary_id,
                               coordinates_for_libmesh_node_id);

  if (block_info.order() > 1)
    handleHigherOrderFESpace(block_info,
                             unique_block_ids,
                             libmesh_element_ids_for_block_id,
                             libmesh_node_ids_for_element_id,
                             coordinates_for_libmesh_node_id);

  // Finalize mesh method is needed to fully finish constructing the mesh.
  FinalizeMesh();
}

void
LibmeshMFEMMesh::buildMFEMVerticesAndElements(
    const int num_elements_in_mesh,
    const LibmeshMFEMBlockInfo & block_info,
    const std::vector<subdomain_id_type> & unique_block_ids,
    const std::map<subdomain_id_type, std::string> & block_ids_to_names,
    const std::vector<boundary_id_type> & unique_side_boundary_ids,
    const std::map<boundary_id_type, std::string> & bound_ids_to_names,
    const std::vector<dof_id_type> & unique_libmesh_corner_node_ids,
    const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
    const NodeIDsForElementID & libmesh_node_ids_for_element_id,
    const NodeIDsForBoundaryID & libmesh_node_ids_for_boundary_id,
    const SideIDsForBoundaryID & libmesh_side_ids_for_boundary_id,
    const BlockIDsForBoundaryID & libmesh_block_ids_for_boundary_id,
    const CoordinatesForNodeID & coordinates_for_libmesh_node_id)
{
  // Set dimensions.
  Dim = spaceDim = block_info.dimension();

  // Create the vertices.
  buildMFEMVertices(unique_libmesh_corner_node_ids, coordinates_for_libmesh_node_id);

  // Create the mesh elements.
  buildMFEMElements(num_elements_in_mesh,
                    block_info,
                    unique_block_ids,
                    block_ids_to_names,
                    libmesh_element_ids_for_block_id,
                    libmesh_node_ids_for_element_id);

  // Create the boundary elements.
  buildMFEMBoundaryElements(block_info,
                            unique_side_boundary_ids,
                            bound_ids_to_names,
                            libmesh_node_ids_for_boundary_id,
                            libmesh_side_ids_for_boundary_id,
                            libmesh_block_ids_for_boundary_id);
}

void
LibmeshMFEMMesh::buildMFEMVertices(const std::vector<dof_id_type> & unique_libmesh_corner_node_ids,
                                   const CoordinatesForNodeID & coordinates_for_libmesh_node_id)
{
  _mfem_vertex_index_for_libmesh_corner_node_id.clear();

  NumOfVertices = unique_libmesh_corner_node_ids.size();
  vertices.SetSize(NumOfVertices);

  // Iterate over the global IDs of each unqiue corner node from the MOOSE mesh.
  int ivertex = 0;
  for (const auto libmesh_node_id : unique_libmesh_corner_node_ids)
  {
    // Get the xyz coordinates associated with the libmesh corner node.
    auto & coordinates = libmesh_map_find(coordinates_for_libmesh_node_id, libmesh_node_id);

    // Set the components used by a mesh of this dimension.
    for (const auto d : make_range(Dim))
      vertices[ivertex](d) = coordinates[d];

    _mfem_vertex_index_for_libmesh_corner_node_id[libmesh_node_id] = ivertex;
    ivertex++;
  }
}

void
LibmeshMFEMMesh::buildMFEMElements(
    const int num_elements_in_mesh,
    const LibmeshMFEMBlockInfo & block_info,
    const std::vector<subdomain_id_type> & unique_block_ids,
    const std::map<subdomain_id_type, std::string> & block_ids_to_names,
    const ElementIDsForBlockID & element_ids_for_block_id,
    const NodeIDsForElementID & node_ids_for_element_id)
{
  _mfem_element_id_for_libmesh_element_id.clear();

  // Set mesh elements.
  NumOfElements = num_elements_in_mesh;
  elements.SetSize(num_elements_in_mesh);

  int ielement = 0;
  for (const auto block_id : unique_block_ids)
  {
    // Get the element type associated with the block.
    auto & block_element = block_info.blockElement(block_id);

    std::vector<int> renumbered_vertex_ids(block_element.num_corner_nodes);

    auto & element_ids = libmesh_map_find(element_ids_for_block_id, block_id);

    for (const auto element_id : element_ids) // Iterate over elements in block.
    {
      auto & libmesh_node_ids = libmesh_map_find(node_ids_for_element_id, element_id);

      // Iterate over ONLY the corner nodes in the element.
      for (const auto ivertex : make_range(block_element.num_corner_nodes))
      {
        const auto libmesh_node_id = libmesh_node_ids[ivertex];

        // Map from the corner libmesh node --> corresponding mfem vertex.
        renumbered_vertex_ids[ivertex] = getMFEMVertexIndex(libmesh_node_id);
      }

      // Map from mfem element id to libmesh element id.
      _mfem_element_id_for_libmesh_element_id[element_id] = ielement;

      elements[ielement++] =
          buildMFEMElement(block_element.mfem_elem_type, renumbered_vertex_ids.data(), block_id);
    }
  }

  for (const auto & pr : block_ids_to_names)
  {
    const auto block_id = pr.first;
    const auto & block_name = pr.second;
    if (!block_name.empty())
    {
      if (!attribute_sets.AttributeSetExists(block_name))
      {
        attribute_sets.CreateAttributeSet(block_name);
      }
      attribute_sets.AddToAttributeSet(block_name, block_id);
    }
  }
}

void
LibmeshMFEMMesh::buildMFEMBoundaryElements(
    const LibmeshMFEMBlockInfo & block_info,
    const std::vector<boundary_id_type> & unique_side_boundary_ids,
    const std::map<boundary_id_type, std::string> & bound_ids_to_names,
    const NodeIDsForBoundaryID & libmesh_node_ids_for_boundary_id,
    const SideIDsForBoundaryID & libmesh_side_ids_for_boundary_id,
    const BlockIDsForBoundaryID & libmesh_block_ids_for_boundary_id)
{
  // Find total number of boundary elements.
  NumOfBdrElements = 0;

  if (unique_side_boundary_ids.empty())
  {
    boundary.SetSize(0);
    return;
  }

  for (const auto boundary_id : unique_side_boundary_ids)
  {
    NumOfBdrElements += libmesh_map_find(libmesh_node_ids_for_boundary_id, boundary_id).size();
  }

  boundary.SetSize(NumOfBdrElements);

  // Iterate over boundary ids.
  int iboundary = 0;
  for (const auto boundary_id : unique_side_boundary_ids)
  {
    auto & all_boundary_node_ids = libmesh_map_find(libmesh_node_ids_for_boundary_id, boundary_id);
    auto & all_boundary_side_ids = libmesh_map_find(libmesh_side_ids_for_boundary_id, boundary_id);
    auto & all_boundary_block_ids =
        libmesh_map_find(libmesh_block_ids_for_boundary_id, boundary_id);

    // Iterate over all elements on boundary.
    for (const auto jelement : index_range(all_boundary_node_ids))
    {
      // Extract the boundary node ids and face id for this boundary element.
      auto & boundary_node_ids = all_boundary_node_ids[jelement];
      auto boundary_face_id = all_boundary_side_ids[jelement];
      auto boundary_block_id = all_boundary_block_ids[jelement];

      // Get the element type and face info.
      auto & boundary_face_info = block_info.blockFace(boundary_block_id, boundary_face_id);

      // Iterate only over the corner nodes and renumber.
      std::vector<int> renumbered_vertex_ids(boundary_face_info.num_corner_nodes);

      for (const auto knode : make_range(boundary_face_info.num_corner_nodes))
      {
        const auto libmesh_node_id = boundary_node_ids[knode];

        // Renumber vertex ("node") IDs so they're contiguous and start from 0.
        renumbered_vertex_ids[knode] = getMFEMVertexIndex(libmesh_node_id);
      }

      boundary[iboundary++] = buildMFEMFaceElement(
          boundary_face_info.mfem_elem_type, renumbered_vertex_ids.data(), boundary_id);
    }
  }

  for (const auto & pr : bound_ids_to_names)
  {
    const auto bound_id = pr.first;
    const auto & bound_name = pr.second;
    if (!bound_name.empty())
    {
      if (!bdr_attribute_sets.AttributeSetExists(bound_name))
      {
        bdr_attribute_sets.CreateAttributeSet(bound_name);
      }
      bdr_attribute_sets.AddToAttributeSet(bound_name, bound_id);
    }
  }
}

mfem::Element *
LibmeshMFEMMesh::buildMFEMElement(const int element_type,
                                  const int * vertex_ids,
                                  const int block_id)
{
  mfem::Element * new_element = nullptr;

  switch (element_type)
  {
    case mfem::Element::Type::SEGMENT:
    {
      new_element = new mfem::Segment(vertex_ids, block_id);
      break;
    }
    case mfem::Element::Type::TRIANGLE:
    {
      new_element = new mfem::Triangle(vertex_ids, block_id);
      break;
    }
    case mfem::Element::Type::QUADRILATERAL:
    {
      new_element = new mfem::Quadrilateral(vertex_ids, block_id);
      break;
    }
    case mfem::Element::Type::TETRAHEDRON:
    {
#ifdef MFEM_USE_MEMALLOC
      new_element = TetMemory.Alloc();
      new_element->SetVertices(vertex_ids);
      new_element->SetAttribute(block_id);
#else
      new_element = new mfem::Tetrahedron(vertex_ids, block_id);
#endif
      break;
    }
    case mfem::Element::Type::HEXAHEDRON:
    {
      new_element = new mfem::Hexahedron(vertex_ids, block_id);
      break;
    }
    case mfem::Element::Type::WEDGE:
    {
      new_element = new mfem::Wedge(vertex_ids, block_id);
      break;
    }
    case mfem::Element::Type::PYRAMID:
    {
      new_element = new mfem::Pyramid(vertex_ids, block_id);
      break;
    }
    default:
    {
      mooseError("Unsupported element type specified.\n");
      break;
    }
  }

  return new_element;
}

mfem::Element *
LibmeshMFEMMesh::buildMFEMFaceElement(const int face_type,
                                      const int * vertex_ids,
                                      const int boundary_id)
{
  mfem::Element * new_face = nullptr;

  switch (face_type)
  {
    case mfem::Element::Type::POINT:
    {
      new_face = new mfem::Point(vertex_ids, boundary_id);
      break;
    }
    case mfem::Element::Type::SEGMENT:
    {
      new_face = new mfem::Segment(vertex_ids, boundary_id);
      break;
    }
    case mfem::Element::Type::TRIANGLE:
    {
      new_face = new mfem::Triangle(vertex_ids, boundary_id);
      break;
    }
    case mfem::Element::Type::QUADRILATERAL:
    {
      new_face = new mfem::Quadrilateral(vertex_ids, boundary_id);
      break;
    }
    default:
    {
      mooseError("Unsupported face type encountered.\n");
      break;
    }
  }

  return new_face;
}

void
LibmeshMFEMMesh::handleHigherOrderFESpace(
    const LibmeshMFEMBlockInfo & block_info,
    const std::vector<subdomain_id_type> & unique_block_ids,
    const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
    const NodeIDsForElementID & libmesh_node_ids_for_element_id,
    const CoordinatesForNodeID & coordinates_for_libmesh_node_id)
{
  // Map from each MFEM node to the libMesh node it was set from.
  std::map<int, dof_id_type> libmesh_node_id_for_mfem_node_id;

  // Call FinalizeTopology. If we call this then we must call Finalize later after
  // we've defined the mesh nodes.
  FinalizeTopology();

  // Define higher-order FE space.
  mfem::FiniteElementCollection * finite_element_collection =
      new mfem::H1_FECollection(block_info.order(), Dim, block_info.basisType(), 0);

  // NB: the specified ordering is byVDIM.
  // byVDim: XYZ, XYZ, XYZ, XYZ,...
  // byNode: XXX..., YYY..., ZZZ...
  mfem::FiniteElementSpace * finite_element_space = new mfem::FiniteElementSpace(
      this, finite_element_collection, spaceDim, mfem::Ordering::byVDIM);

  Nodes = new mfem::GridFunction(finite_element_space);
  Nodes->MakeOwner(finite_element_collection); // Nodes will destroy 'finite_element_collection'
  own_nodes = 1;                               // and 'finite_element_space'

  // Iterate over blocks and libmesh elements.
  for (auto block_id : unique_block_ids)
  {
    auto & libmesh_element_ids = libmesh_map_find(libmesh_element_ids_for_block_id, block_id);

    // Find the element type.
    auto & block_element = block_info.blockElement(block_id);

    // Iterate over elements in the block.
    for (auto libmesh_element_id : libmesh_element_ids)
    {
      auto mfem_element_id = getMFEMElementID(libmesh_element_id);

      // Get vector containing ALL node global IDs for element.
      auto & libmesh_node_ids =
          libmesh_map_find(libmesh_node_ids_for_element_id, libmesh_element_id);

      // Sets DOF array for element. Higher-order (second-order) elements contain
      // additional nodes between corner nodes.
      mfem::Array<int> dofs;
      finite_element_space->GetElementDofs(mfem_element_id, dofs);

      // Iterate over dofs array.
      for (const auto j : make_range(block_element.num_nodes))
      {
        const int mfem_node_id = dofs[j];

        // Find the libmesh node ID:
        // NB: the map is 1-based to we need to subtract 1.
        const int libmesh_node_index = block_element.mfem_to_libmesh[j] - 1;
        const auto libmesh_node_id = libmesh_node_ids[libmesh_node_index];

        libmesh_node_id_for_mfem_node_id[mfem_node_id] = libmesh_node_id;

        // Extract node's coordinates:
        auto & coordinates = libmesh_map_find(coordinates_for_libmesh_node_id, libmesh_node_id);

        SetNode(dofs[j], coordinates.data());
      }

      // Set any nodes not present in the libMesh data by interpolating the
      // nodes which are present.
      for (const auto i : index_range(block_element.additional_points))
      {
        const auto & weights = block_element.additional_points[i];
        std::array<mfem::real_t, 3> coordinates{0., 0., 0.};
        for (const auto j : make_range(block_element.num_nodes))
        {
          const auto & c = libmesh_map_find(coordinates_for_libmesh_node_id, libmesh_node_ids[j]);
          const mfem::real_t w = weights[j];
          coordinates[0] += w * c[0];
          coordinates[1] += w * c[1];
          coordinates[2] += w * c[2];
        }
        SetNode(dofs[block_element.num_nodes + i], coordinates.data());
      }
    }
  }

  /**
   * Ensure that there is a one-to-one mapping between libmesh and mfem node ids.
   * All coordinates should match. If this does not occur then it suggests that
   * there is a problem with the higher-order transfer.
   */
  verifyUniqueMappingBetweenLibmeshAndMFEMNodes(block_info,
                                                libmesh_node_ids_for_element_id,
                                                coordinates_for_libmesh_node_id,
                                                libmesh_node_id_for_mfem_node_id);
}

void
LibmeshMFEMMesh::verifyUniqueMappingBetweenLibmeshAndMFEMNodes(
    const LibmeshMFEMBlockInfo & block_info,
    const NodeIDsForElementID & libmesh_node_ids_for_element_id,
    const CoordinatesForNodeID & coordinates_for_libmesh_node_id,
    const std::map<int, dof_id_type> & libmesh_node_id_for_mfem_node_id)
{
  const auto * finite_element_space = GetNodalFESpace();
  if (!finite_element_space)
  {
    mooseError("No nodal FE space.");
  }

  // Create a set of all unique libmesh node ids.
  std::set<dof_id_type> libmesh_node_ids;

  for (auto & key_value : libmesh_node_ids_for_element_id)
  {
    const auto & libmesh_node_ids_for_element = key_value.second;

    for (const auto libmesh_node_id : libmesh_node_ids_for_element)
    {
      libmesh_node_ids.insert(libmesh_node_id);
    }
  }

  // This needs to be initialised, so we don't have garbage in the 2nd
  // and 3rd elements when getting 1D and 2D data.
  double mfem_coordinates[3] = {0., 0., 0.};

  for (const auto ielement : make_range(NumOfElements))
  {
    mfem::Array<int> mfem_dofs;
    const auto & element_info = block_info.blockElement(GetAttribute(ielement));
    finite_element_space->GetElementDofs(ielement, mfem_dofs);

    for (const auto j : make_range(mfem_dofs.Size()))
    {
      int mfem_dof = mfem_dofs[j];
      GetNode(mfem_dof, mfem_coordinates);

      if (j < element_info.num_nodes)
      {
        const auto libmesh_node_id = libmesh_map_find(libmesh_node_id_for_mfem_node_id, mfem_dof);

        // Remove from set.
        libmesh_node_ids.erase(libmesh_node_id);

        auto & libmesh_coordinates =
            libmesh_map_find(coordinates_for_libmesh_node_id, libmesh_node_id);

        // The nodes were copied from the libMesh coordinates, so they should match exactly.
        if (!std::equal(mfem_coordinates, mfem_coordinates + spaceDim, libmesh_coordinates.begin()))
        {
          mooseError("Non-matching coordinates detected for libmesh node ",
                     libmesh_node_id,
                     " and MFEM node ",
                     mfem_dof,
                     " for MFEM element ",
                     ielement,
                     ".");
        }
      }
    }
  }

  // Check how many elements remain in set of libmesh element ids. Ideally,
  // there should be none left indicating that we've referenced every single
  // element in the set.
  if (libmesh_node_ids.size() != 0)
  {
    mooseError("There are ",
               libmesh_node_ids.size(),
               " unpaired libmesh node ids. No one-to-one mapping exists!");
  }
}

#endif
