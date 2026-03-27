#pragma once
#include "LibmeshMFEMMesh.h"
#include <mfem/mesh/element.hpp>

// Function prototypes:
static bool
coordinatesMatch(const double * primary, const double * secondary, const double tolerance = 0.01);

/**
 * Initializer for 1st order elements.
 */
LibmeshMFEMMesh::LibmeshMFEMMesh(
    const int num_elements_in_mesh,
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_block_ids,
    const std::map<libMesh::subdomain_id_type, std::string> & block_ids_to_names,
    const std::vector<int> & unique_side_boundary_ids,
    const std::map<libMesh::boundary_id_type, std::string> & bound_ids_to_names,
    const std::vector<int> & unique_libmesh_corner_node_ids,
    const std::map<int, std::vector<int>> & libmesh_element_ids_for_block_id,
    const std::map<int, std::vector<int>> & libmesh_node_ids_for_element_id,
    const std::map<int, std::vector<std::vector<unsigned int>>> & libmesh_node_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_side_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_block_ids_for_boundary_id,
    const std::map<int, std::array<double, 3>> & coordinates_for_libmesh_node_id)
{
  if (block_info.order() != 1)
  {
    mooseError("1st order initializer called for order ", block_info.order(), ".");
  }

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

  // Finalize mesh method is needed to fully finish constructing the mesh.
  FinalizeMesh();
}

/**
 * Initializer for higher-order elements.
 */
LibmeshMFEMMesh::LibmeshMFEMMesh(
    const int num_elements_in_mesh,
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_block_ids,
    const std::map<libMesh::subdomain_id_type, std::string> & block_ids_to_names,
    const std::vector<int> & unique_side_boundary_ids,
    const std::map<libMesh::boundary_id_type, std::string> & bound_ids_to_names,
    const std::vector<int> & unique_libmesh_corner_node_ids,
    const std::map<int, std::vector<int>> & libmesh_element_ids_for_block_id,
    const std::map<int, std::vector<int>> & libmesh_node_ids_for_element_id,
    const std::map<int, std::vector<std::vector<unsigned int>>> & libmesh_node_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_side_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_block_ids_for_boundary_id,
    const std::map<int, std::array<double, 3>> & coordinates_for_libmesh_node_id,
    std::map<int, int> & libmesh_node_id_for_mfem_node_id,
    std::map<int, int> & mfem_node_id_for_libmesh_node_id)
{
  if (block_info.order() < 2)
  {
    mooseError("Higher-order initializer called for order ", block_info.order(), ".");
  }

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

  handleHigherOrderFESpace(block_info,
                           unique_block_ids,
                           libmesh_element_ids_for_block_id,
                           libmesh_node_ids_for_element_id,
                           coordinates_for_libmesh_node_id,
                           libmesh_node_id_for_mfem_node_id,
                           mfem_node_id_for_libmesh_node_id);

  FinalizeMesh();
}

LibmeshMFEMMesh::LibmeshMFEMMesh(std::string mesh_fname,
                                 int generate_edges,
                                 int refine,
                                 bool fix_orientation)
  : _mfem_element_id_for_libmesh_element_id{}, _mfem_vertex_index_for_libmesh_corner_node_id{}
{
  SetEmpty();

  mfem::named_ifgzstream mesh_fstream(mesh_fname);
  if (!mesh_fstream) // TODO: - can this be nullptr?
  {
    mooseError("Failed to read '" + mesh_fname + "'\n");
  }
  else
  {
    Load(mesh_fstream, generate_edges, refine, fix_orientation);
  }
}

void
LibmeshMFEMMesh::buildMFEMVerticesAndElements(
    const int num_elements_in_mesh,
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_block_ids,
    const std::map<libMesh::subdomain_id_type, std::string> & block_ids_to_names,
    const std::vector<int> & unique_side_boundary_ids,
    const std::map<libMesh::boundary_id_type, std::string> & bound_ids_to_names,
    const std::vector<int> & unique_libmesh_corner_node_ids,
    const std::map<int, std::vector<int>> & libmesh_element_ids_for_block_id,
    const std::map<int, std::vector<int>> & libmesh_node_ids_for_element_id,
    const std::map<int, std::vector<std::vector<unsigned int>>> & libmesh_node_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_side_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_block_ids_for_boundary_id,
    const std::map<int, std::array<double, 3>> & coordinates_for_libmesh_node_id)
{
  // Set dimensions.
  // TODO: double check that Dim and spaceDim are always equal for LibMesh
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
LibmeshMFEMMesh::buildMFEMVertices(
    const std::vector<int> & unique_libmesh_corner_node_ids,
    const std::map<int, std::array<double, 3>> & coordinates_for_libmesh_node_id)
{
  _mfem_vertex_index_for_libmesh_corner_node_id.clear();

  NumOfVertices = unique_libmesh_corner_node_ids.size();
  vertices.SetSize(NumOfVertices);

  // Iterate over the global IDs of each unqiue corner node from the MOOSE mesh.
  const bool nonzero_y_component = (Dim > 1);
  const bool use_z_component = (Dim == 3);

  int ivertex = 0;
  for (int libmesh_node_id : unique_libmesh_corner_node_ids)
  {
    // Get the xyz coordinates associated with the libmesh corner node.
    auto & coordinates = coordinates_for_libmesh_node_id.at(libmesh_node_id);

    // FIXME: Look at mfem::Mesh::Make1D() to see if we can set just one component of the coordinate

    // Set xyz components.
    vertices[ivertex](0) = coordinates[0];

    if (nonzero_y_component)
    {
      vertices[ivertex](1) = coordinates[1];
    }
    else
    {
      vertices[ivertex](2) = 0.;
    }

    if (use_z_component)
    {
      vertices[ivertex](2) = coordinates[2];
    }

    _mfem_vertex_index_for_libmesh_corner_node_id[libmesh_node_id] = ivertex;
    ivertex++;
  }
}

void
LibmeshMFEMMesh::buildMFEMElements(
    const int num_elements_in_mesh,
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_block_ids,
    const std::map<libMesh::subdomain_id_type, std::string> & block_ids_to_names,
    const std::map<int, std::vector<int>> & element_ids_for_block_id,
    const std::map<int, std::vector<int>> & node_ids_for_element_id)
{
  _mfem_element_id_for_libmesh_element_id.clear();

  // Set mesh elements.
  NumOfElements = num_elements_in_mesh;
  elements.SetSize(num_elements_in_mesh);

  int ielement = 0;
  for (int block_id : unique_block_ids)
  {
    // Get the element type associated with the block.
    auto & block_element = block_info.blockElement(block_id);

    std::vector<int> renumbered_vertex_ids(block_element.num_corner_nodes);

    auto & element_ids = element_ids_for_block_id.at(block_id);

    for (int element_id : element_ids) // Iterate over elements in block.
    {
      auto & libmesh_node_ids = node_ids_for_element_id.at(element_id);

      // Iterate over ONLY the corner nodes in the element.
      for (int ivertex = 0; ivertex < block_element.num_corner_nodes; ivertex++)
      {
        const int libmesh_node_id = libmesh_node_ids[ivertex];

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
    std::cout << "Block '" << block_name << "' has id " << block_id << "\n";
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
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_side_boundary_ids,
    const std::map<libMesh::boundary_id_type, std::string> & bound_ids_to_names,
    const std::map<int, std::vector<std::vector<unsigned int>>> & libmesh_node_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_side_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & libmesh_block_ids_for_boundary_id)
{
  // Find total number of boundary elements.
  NumOfBdrElements = 0;

  if (unique_side_boundary_ids.empty())
  {
    boundary.SetSize(0);
    return;
  }

  for (int boundary_id : unique_side_boundary_ids)
  {
    NumOfBdrElements += libmesh_node_ids_for_boundary_id.at(boundary_id).size();
  }

  boundary.SetSize(NumOfBdrElements);

  // Iterate over boundary ids.
  int iboundary = 0;
  for (int boundary_id : unique_side_boundary_ids)
  {
    auto & all_boundary_node_ids = libmesh_node_ids_for_boundary_id.at(boundary_id);
    auto & all_boundary_side_ids = libmesh_side_ids_for_boundary_id.at(boundary_id);
    auto & all_boundary_block_ids = libmesh_block_ids_for_boundary_id.at(boundary_id);

    // Iterate over all elements on boundary.
    for (int jelement = 0; jelement < (int)all_boundary_node_ids.size(); jelement++)
    {
      // Extract the boundary node ids and face id for this boundary element.
      auto & boundary_node_ids = all_boundary_node_ids[jelement];
      auto boundary_face_id = all_boundary_side_ids[jelement];
      auto boundary_block_id = all_boundary_block_ids[jelement];

      // Get the element type and face info.
      auto & boundary_face_info = block_info.blockFace(boundary_block_id, boundary_face_id);

      // Iterate only over the corner nodes and renumber.
      std::vector<int> renumbered_vertex_ids(boundary_face_info.num_corner_nodes);

      for (int knode = 0; knode < boundary_face_info.num_corner_nodes; knode++)
      {
        const int libmesh_node_id = boundary_node_ids[knode];

        // Renumber vertex ("node") IDs so they're contiguous and start from 0.
        renumbered_vertex_ids[knode] = getMFEMVertexIndex(libmesh_node_id);
      }

      boundary[iboundary++] = buildMFEMFaceElement(
          boundary_face_info.mfem_elem_type, renumbered_vertex_ids.data(), boundary_block_id);
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

  // FIXME: Need to add support for SEG4, QUAD8, TRI7, HEX20, TET14, WEDGE15, PYRAMID13

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
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_block_ids,
    const std::map<int, std::vector<int>> & libmesh_element_ids_for_block_id,
    const std::map<int, std::vector<int>> & libmesh_node_ids_for_element_id,
    const std::map<int, std::array<double, 3>> & coordinates_for_libmesh_node_id,
    std::map<int, int> & libmesh_node_id_for_mfem_node_id,
    std::map<int, int> & mfem_node_id_for_libmesh_node_id)
{
  // Verify that this is indeed a second-order element.
  if (block_info.order() < 2)
  {
    return;
  }

  // Add a warning for 2D second-order elements but proceed.
  // if (block_info.dimension() < 3)
  // {
  //   mooseWarning("'", __func__, "' has not been tested with higher-order 1D or 2D elements.");
  // }

  // Clear second order maps.
  libmesh_node_id_for_mfem_node_id.clear();
  mfem_node_id_for_libmesh_node_id.clear();

  // Call FinalizeTopology. If we call this then we must call Finalize later after
  // we've defined the mesh nodes.
  FinalizeTopology();

  // Define higher-order FE space.
  mfem::FiniteElementCollection * finite_element_collection =
      new mfem::H1_FECollection(block_info.order(), Dim, block_info.basisType(), 0);

  // NB: the specified ordering is byVDIM.
  // byVDim: XYZ, XYZ, XYZ, XYZ,...
  // byNode: XXX..., YYY..., ZZZ...
  mfem::FiniteElementSpace * finite_element_space =
      new mfem::FiniteElementSpace(this, finite_element_collection, spaceDim);

  Nodes = new mfem::GridFunction(finite_element_space);
  Nodes->MakeOwner(finite_element_collection); // Nodes will destroy 'finite_element_collection'
  own_nodes = 1;                               // and 'finite_element_space'

  // Iterate over blocks and libmesh elements.
  for (auto block_id : unique_block_ids)
  {
    auto & libmesh_element_ids = libmesh_element_ids_for_block_id.at(block_id);

    // Find the element type.
    auto & block_element = block_info.blockElement(block_id);

    // Iterate over elements in the block.
    for (auto libmesh_element_id : libmesh_element_ids)
    {
      auto mfem_element_id = getMFEMElementID(libmesh_element_id);

      // Get vector containing ALL node global IDs for element.
      auto & libmesh_node_ids = libmesh_node_ids_for_element_id.at(libmesh_element_id);

      // Sets DOF array for element. Higher-order (second-order) elements contain
      // additional nodes between corner nodes.
      mfem::Array<int> dofs;
      finite_element_space->GetElementDofs(mfem_element_id, dofs);

      // Iterate over dofs array.
      for (int j = 0; j < block_element.num_nodes; j++)
      {
        const int mfem_node_id = dofs[j];

        // Find the libmesh node ID:
        // NB: the map is 1-based to we need to subtract 1.
        const int libmesh_node_index = block_element.mfem_to_libmesh[j] - 1;
        const int libmesh_node_id = libmesh_node_ids[libmesh_node_index];

        // Update two-way map:
        libmesh_node_id_for_mfem_node_id[mfem_node_id] = libmesh_node_id;
        mfem_node_id_for_libmesh_node_id[libmesh_node_id] = mfem_node_id;

        // Extract node's coordinates:
        auto & coordinates = coordinates_for_libmesh_node_id.at(libmesh_node_id);

        SetNode(dofs[j], coordinates.data());
      }

      // Set any nodes not present in the libMesh data by interpolating the
      // nodes which are present.
      for (unsigned int i = 0; i < block_element.additional_points.size(); i++)
      {
        const auto & weights = block_element.additional_points[i];
        std::array<mfem::real_t, 3> coordinates{0., 0., 0.};
        for (int j = 0; j < block_element.num_nodes; ++j)
        {
          const auto & c = coordinates_for_libmesh_node_id.at(j);
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
                                                unique_block_ids,
                                                libmesh_element_ids_for_block_id,
                                                libmesh_node_ids_for_element_id,
                                                coordinates_for_libmesh_node_id,
                                                libmesh_node_id_for_mfem_node_id);
}

void
LibmeshMFEMMesh::verifyUniqueMappingBetweenLibmeshAndMFEMNodes(
    const CubitBlockInfo & block_info,
    const std::vector<int> & unique_block_ids,
    const std::map<int, std::vector<int>> & libmesh_element_ids_for_block_id,
    const std::map<int, std::vector<int>> & libmesh_node_ids_for_element_id,
    const std::map<int, std::array<double, 3>> & coordinates_for_libmesh_node_id,
    const std::map<int, int> & libmesh_node_id_for_mfem_node_id)
{
  const auto * finite_element_space = GetNodalFESpace();
  if (!finite_element_space)
  {
    mooseError("No nodal FE space.");
  }

  // Create a set of all unique libmesh node ids.
  std::set<int> libmesh_node_ids;

  for (auto & key_value : libmesh_node_ids_for_element_id)
  {
    const std::vector<int> & libmesh_node_ids_for_element = key_value.second;

    for (int libmesh_node_id : libmesh_node_ids_for_element)
    {
      libmesh_node_ids.insert(libmesh_node_id);
    }
  }

  // This needs to be initialised, so we don't have garbage in the 2nd
  // and 3rd elements when getting 1D and 2D data.
  double mfem_coordinates[3] = {0., 0., 0.};

  for (int ielement = 0; ielement < NumOfElements; ielement++)
  {
    mfem::Array<int> mfem_dofs;
    const auto & element_info = block_info.blockElement(GetAttribute(ielement));
    finite_element_space->GetElementDofs(ielement, mfem_dofs);

    for (int j = 0; j < mfem_dofs.Size(); j++)
    {
      int mfem_dof = mfem_dofs[j];
      GetNode(mfem_dof, mfem_coordinates);

      if (j < element_info.num_nodes)
      {
        const int libmesh_node_id = libmesh_node_id_for_mfem_node_id.at(mfem_dof);

        // Remove from set.
        libmesh_node_ids.erase(libmesh_node_id);

        auto & libmesh_coordinates = coordinates_for_libmesh_node_id.at(libmesh_node_id);

        if (!coordinatesMatch(libmesh_coordinates.data(), mfem_coordinates))
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

static bool
coordinatesMatch(const double * primary, const double * secondary, const double tolerance)
{
  if (!primary || !secondary || tolerance < 0.0)
  {
    return false;
  }

  for (int i = 0; i < 3; i++)
  {
    if (fabs(primary[i] - secondary[i]) > tolerance)
    {
      return false;
    }
  }

  return true;
}
