//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYZDelaunayGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_netgen_interface.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/parsed_function.h"
#include "libmesh/replicated_mesh.h"

registerMooseObject("MooseApp", XYZDelaunayGenerator);

namespace std
{
template <>
struct hash<std::tuple<libMesh::Point, libMesh::Point, libMesh::Point>>
{
  std::size_t operator()(const std::tuple<libMesh::Point, libMesh::Point, libMesh::Point> & p) const
  {
    std::size_t seed = 0;
    libMesh::boostcopy::hash_combine(seed, std::hash<libMesh::Point>()(std::get<0>(p)));
    libMesh::boostcopy::hash_combine(seed, std::hash<libMesh::Point>()(std::get<1>(p)));
    libMesh::boostcopy::hash_combine(seed, std::hash<libMesh::Point>()(std::get<2>(p)));
    return seed;
  }
};

} // namespace std

InputParameters
XYZDelaunayGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");

  params.addRequiredParam<MeshGeneratorName>(
      "boundary",
      "The input MeshGenerator defining the output outer boundary. The input mesh can either "
      "include 3D volume elements or 2D surface elements.");

  params.addParam<SubdomainName>("output_subdomain_name",
                                 "Subdomain name to set on new triangles.");

  params.addParam<BoundaryName>(
      "output_boundary",
      "Boundary name to set on new outer boundary.  Default ID: 0 if no hole meshes are stitched; "
      "or maximum boundary ID of all the stitched hole meshes + 1.");
  params.addParam<std::vector<BoundaryName>>(
      "hole_boundaries",
      "Boundary names to set on holes.  Default IDs are numbered up from 1 if no hole meshes are "
      "stitched; or from maximum boundary ID of all the stitched hole meshes + 2.");

  params.addParam<bool>("smooth_triangulation",
                        false,
                        "Whether to do Laplacian mesh smoothing on the generated triangles.");
  params.addParam<std::vector<MeshGeneratorName>>(
      "holes",
      std::vector<MeshGeneratorName>(),
      "The MeshGenerators that define mesh holes. A hole mesh must contain 3D volume elements and "
      "its external surface works as the closed manifold that defines the hole.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");
  params.addParam<bool>("convert_holes_for_stitching",
                        false,
                        "Whether to convert the 3D hole meshes with non-TRI3 surface sides into "
                        "all-TET4 meshes to allow stitching.");

  params.addRangeCheckedParam<Real>(
      "desired_volume",
      0,
      "desired_volume>=0",
      "Desired (maximum) tetrahedral volume, or 0 to skip uniform refinement");

  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh hole stitching should have verbose output.");

  params.addClassDescription(
      "Creates tetrahedral 3D meshes within boundaries defined by input meshes.");

  return params;
}

XYZDelaunayGenerator::XYZDelaunayGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _bdy_ptr(getMesh("boundary")),
    _desired_volume(getParam<Real>("desired_volume")),
    _output_subdomain_id(0),
    _smooth_tri(getParam<bool>("smooth_triangulation")),
    _hole_ptrs(getMeshes("holes")),
    _stitch_holes(getParam<std::vector<bool>>("stitch_holes")),
    _convert_holes_for_stitching(getParam<bool>("convert_holes_for_stitching")),
    _algorithm(parameters.get<MooseEnum>("algorithm")),
    _verbose_stitching(parameters.get<bool>("verbose_stitching"))
{
  if (!_stitch_holes.empty() && _stitch_holes.size() != _hole_ptrs.size())
    paramError("stitch_holes", "Need one stitch_holes entry per hole, if specified.");

  if (isParamValid("hole_boundaries"))
  {
    auto & hole_boundaries = getParam<std::vector<BoundaryName>>("hole_boundaries");
    if (hole_boundaries.size() != _hole_ptrs.size())
      paramError("hole_boundaries", "Need one hole_boundaries entry per hole, if specified.");
  }
}

std::unique_ptr<MeshBase>
XYZDelaunayGenerator::generate()
{
#ifdef LIBMESH_HAVE_NETGEN
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_bdy_ptr));

  // The libMesh Netgen removes all the sideset info
  // But it keeps some nodeset info for the retained nodes
  // We need to clear these nodeset info as they could be annoying
  mesh->get_boundary_info().clear_boundary_node_ids();

  // Get ready to triangulate its boundary
  libMesh::NetGenMeshInterface ngint(*mesh);

  ngint.smooth_after_generating() = _smooth_tri;

  ngint.desired_volume() = _desired_volume;

  // The hole meshes will be used for hole boundary identification and optionally for stitching.
  // if a hole mesh contains 3D volume elements but has non-TRI3 surface side elements, it cannot be
  // used directly for stitching. But it can be converted into a all-TET4 mesh to support hole
  // boundary identification
  for (auto hole_i : index_range(_hole_ptrs))
  {
    UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(**_hole_ptrs[hole_i]);
    libMesh::MeshSerializer serial_hole(hole_mesh);
    // Check the dimension of the hole mesh
    // We do not need to worry about element order here as libMesh checks it
    std::set<ElemType> hole_elem_types;
    std::set<unsigned short> hole_elem_dims;
    for (auto elem : hole_mesh.element_ptr_range())
    {
      hole_elem_dims.emplace(elem->dim());

      // For a 3D element, we need to check the surface side element type instead of the element
      // type
      if (elem->dim() == 3)
        for (auto s : make_range(elem->n_sides()))
        {
          if (!elem->neighbor_ptr(s))
            hole_elem_types.emplace(elem->side_ptr(s)->type());
        }
      // For a non-3D element, we just need to record the element type
      // but an error will be thrown later
      else
        hole_elem_types.emplace(elem->type());
    }
    if (hole_elem_dims.size() != 1)
      paramError("holes", "All elements in a hole mesh must have the same dimension (3D).");
    else if (*hole_elem_dims.begin() == 3)
    {
      // For 3D meshes, if there are non-TRI3 surface side elements
      // (1) if no stitching is needed, we can just convert the whole mesh into TET to facilitate
      // boundary identification (2) if stitching is needed, we can still convert and stitch, but
      // that would modify the input hole mesh
      if (*hole_elem_types.begin() != ElemType::TRI3)
      {
        if (_stitch_holes.size() && _stitch_holes[hole_i] && !_convert_holes_for_stitching)
          paramError("holes",
                     "3D hole meshes with non-TRI3 surface elements cannot be stitched without "
                     "converting them to TET4. Consider setting convert_holes_for_stitching=true.");
        else
          MeshTools::Modification::all_tri(**_hole_ptrs[hole_i]);
      }
    }
    else if (*hole_elem_dims.begin() == 2)
    {
      // 2D hole meshes are currently not supported
      paramError("holes",
                 "2D hole meshes are currently not supported. Consider using separate "
                 "XYZDelaunayGenerators to create 3D hole meshes.");
    }
    else
      paramError("holes", "All elements in a hole mesh must 3D.");
  }

  std::unique_ptr<std::vector<std::unique_ptr<UnstructuredMesh>>> ngholes =
      std::make_unique<std::vector<std::unique_ptr<UnstructuredMesh>>>();

  // The libMesh Netgen interface will modify hole meshes in-place, so
  // we make copies to pass in.
  for (std::unique_ptr<MeshBase> * hole_ptr : _hole_ptrs)
  {
    // How did we never add a ReplicatedMesh(MeshBase&) constructor in
    // libMesh?
    const UnstructuredMesh & hole = dynamic_cast<UnstructuredMesh &>(**hole_ptr);
    ngholes->push_back(std::make_unique<ReplicatedMesh>(hole));
  }

  if (!_hole_ptrs.empty())
    ngint.attach_hole_list(std::move(ngholes));

  ngint.triangulate();

  if (isParamValid("output_subdomain_name"))
  {
    auto output_subdomain_name = getParam<SubdomainName>("output_subdomain_name");
    _output_subdomain_id = MooseMeshUtils::getSubdomainID(output_subdomain_name, *mesh);

    if (_output_subdomain_id == Elem::invalid_subdomain_id)
    {
      // We'll probably need to make a new ID, then
      _output_subdomain_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

      // But check the hole meshes for our output subdomain name too
      for (auto & hole_ptr : _hole_ptrs)
      {
        auto possible_sbdid = MooseMeshUtils::getSubdomainID(output_subdomain_name, **hole_ptr);
        // Huh, it was in one of them
        if (possible_sbdid != Elem::invalid_subdomain_id)
        {
          _output_subdomain_id = possible_sbdid;
          break;
        }
        _output_subdomain_id =
            std::max(_output_subdomain_id, MooseMeshUtils::getNextFreeSubdomainID(**hole_ptr));
      }

      mesh->subdomain_name(_output_subdomain_id) = output_subdomain_name;
    }
  }

  if (_smooth_tri || _output_subdomain_id)
    for (auto elem : mesh->element_ptr_range())
    {
      elem->subdomain_id() = _output_subdomain_id;

      // I do not trust Laplacian mesh smoothing not to invert
      // elements near reentrant corners.  Eventually we'll add better
      // smoothing options, but even those might have failure cases.
      // Better to always do extra tests here than to ever let users
      // try to run on a degenerate mesh.
      if (elem->is_flipped())
      {
        if (_smooth_tri)
          mooseError("Inverted element found in triangulation.\n"
                     "Laplacian smoothing can create these at reentrant corners; disable it?");
        else
          mooseError("Unexplained inverted element found in triangulation.\n");
      }
    }

  const bool use_binary_search = (_algorithm == "BINARY");

  // The hole meshes are specified by the user, so they could have any
  // BCID or no BCID or any combination of BCIDs on their outer
  // boundary, so we'll have to set our own BCID to use for stitching
  // there.  We'll need to check all the holes for used BCIDs, if we
  // want to pick a new ID on hole N that doesn't conflict with any
  // IDs on hole M < N (or with the IDs on the new triangulation)

  // The Netgen generated mesh does not have hole and output boundary ids,
  // which is different from its 2D counterpart.
  // So, we need to assign boundary ids to the hole boundaries and output boundary
  // By default, we assign the hole boundary ids to be 1,2,..., N
  // and the output boundary id to be 0
  // We will need a different set of boundary ids if we are stitching holes

  // end_bcid is one more than the maximum of default output boundary id and hole boundary ids
  const boundary_id_type end_bcid = _hole_ptrs.size() + 1;

  // For the hole meshes that need to be stitched, we would like to make sure the hole boundary ids
  // and output boundary id are not conflicting with the existing boundary ids of the hole meshes to
  // be stitched.
  BoundaryID free_boundary_id = 0;
  // We need to get a free boundary id that is larger than the existing boundary ids in any hole
  // meshes
  if (_stitch_holes.size())
  {
    for (auto hole_i : index_range(_hole_ptrs))
    {
      if (_stitch_holes[hole_i])
      {
        free_boundary_id =
            std::max(free_boundary_id, MooseMeshUtils::getNextFreeBoundaryID(**_hole_ptrs[hole_i]));
        (*_hole_ptrs[hole_i])->comm().max(free_boundary_id);
      }
    }
  }
  // new_hole_bcid is used to ensure the boundary ids used for stitching have no conflicts
  // If we shift the default hole boundary ids and output boundary id by free_boundary_id, we need
  // to have new_hole_bcid that is larger than any existing boundary ids in the hole meshes and the
  // Netgen generated mesh
  boundary_id_type new_hole_bcid = end_bcid + free_boundary_id;

  auto hole_boundaries = isParamValid("hole_boundaries")
                             ? getParam<std::vector<BoundaryName>>("hole_boundaries")
                             : std::vector<BoundaryName>();
  const BoundaryName output_boundary =
      isParamValid("output_boundary") ? getParam<BoundaryName>("output_boundary") : BoundaryName();

  // if outer boundary id is not specified by a numeric output_boundary, we just assign
  // free_boundary_id to the output boundary
  const std::vector<BoundaryID> output_boundary_id =
      isParamValid("output_boundary")
          ? (MooseUtils::isDigits(output_boundary)
                 ? std::vector<BoundaryID>(
                       1, MooseMeshUtils::getIDFromName<BoundaryName, BoundaryID>(output_boundary))
                 : std::vector<BoundaryID>(1, free_boundary_id))
          : std::vector<BoundaryID>();
  // Similarly, set the hole boundary ids one by one
  std::vector<BoundaryID> hole_boundary_ids;
  if (isParamValid("hole_boundaries"))
    for (auto h : index_range(hole_boundaries))
      hole_boundary_ids.push_back(
          MooseUtils::isDigits(hole_boundaries[h])
              ? MooseMeshUtils::getIDFromName<BoundaryName, BoundaryID>(hole_boundaries[h])
              : h + 1 + free_boundary_id);

  // if large ids are used in the hole boundaries or the output boundary,
  // we need to make sure the new_hole_bcid is larger than them
  if (hole_boundary_ids.size())
    for (auto h : index_range(_hole_ptrs))
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(hole_boundary_ids[h] + 1));
  if (output_boundary_id.size())
    new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(output_boundary_id[0] + 1));

  bool doing_stitching = false;

  for (auto hole_i : index_range(_hole_ptrs))
  {
    const MeshBase & hole_mesh = **_hole_ptrs[hole_i];
    auto & hole_boundary_info = hole_mesh.get_boundary_info();
    const std::set<boundary_id_type> & local_hole_bcids = hole_boundary_info.get_boundary_ids();

    if (!local_hole_bcids.empty())
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(*local_hole_bcids.rbegin() + 1));
    hole_mesh.comm().max(new_hole_bcid);

    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
      doing_stitching = true;
  }
  // Now we can define boundaries to assist with stitching
  const boundary_id_type inner_bcid = new_hole_bcid + 1;

  // libMesh mesh stitching still requires a serialized mesh, and it's
  // cheaper to do that once than to do it once-per-hole
  libMesh::MeshSerializer serial(*mesh, doing_stitching);

  // We'll be looking for any sides that match between hole meshes and
  // the newly triangulated mesh, to apply bcids accordingly.  We
  // can't key on Elem::key() here, because that depends on node ids
  // that differ from mesh to mesh.  We can't use centroids here,
  // because that depends on rounding error in order of operations
  // that can differ from mesh to mesh.  The node locations themselves
  // should always match up exactly, though, so let's use (sorted!)
  // tuples of those to map from nodes to elements and side numbers.
  // Also added a Boolean to check if the face is already used
  std::unordered_map<std::tuple<Point, Point, Point>,
                     std::pair<std::pair<Elem *, unsigned int>, bool>>
      mesh_faces;

  auto sorted_point_tuple = [](Elem & elem, unsigned int side)
  {
    std::vector<unsigned int> nodes_on_side = elem.nodes_on_side(side);
    libmesh_assert_equal_to(nodes_on_side.size(), 3);
    std::vector<Point> p(3);
    for (auto i : index_range(p))
      p[i] = elem.point(nodes_on_side[i]);
    if (p[0] < p[1])
    {
      if (p[1] < p[2])
        return std::make_tuple(p[0], p[1], p[2]);
      else if (p[0] < p[2])
        return std::make_tuple(p[0], p[2], p[1]);
      else
        return std::make_tuple(p[2], p[0], p[1]);
    }
    else
    {
      if (p[0] < p[2])
        return std::make_tuple(p[1], p[0], p[2]);
      else if (p[1] < p[2])
        return std::make_tuple(p[1], p[2], p[0]);
      else
        return std::make_tuple(p[2], p[1], p[0]);
    }
  };

  if (!_hole_ptrs.empty())
    for (auto elem : mesh->element_ptr_range())
      for (auto s : make_range(elem->n_sides()))
        if (!elem->neighbor_ptr(s))
        {
          auto points = sorted_point_tuple(*elem, s);
          libmesh_assert(!mesh_faces.count(points));
          mesh_faces.emplace(points, std::make_pair(std::make_pair(elem, s), false));
        }

  auto & mesh_boundary_info = mesh->get_boundary_info();

  // Define a reference map variable for subdomain map
  auto & main_subdomain_map = mesh->set_subdomain_name_map();
  for (auto hole_i : index_range(_hole_ptrs))
  {
    UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(**_hole_ptrs[hole_i]);
    auto & hole_boundary_info = hole_mesh.get_boundary_info();

    // Our algorithm here requires a serialized Mesh.  To avoid
    // redundant serialization and deserialization (libMesh
    // MeshedHole and stitch_meshes still also require
    // serialization) we'll do the serialization up front.
    libMesh::MeshSerializer serial_hole(hole_mesh);

    // We'll look for any sides that match between the hole mesh and
    // the newly triangulated mesh, and apply bcids accordingly.
    for (auto elem : hole_mesh.element_ptr_range())
      for (auto s : make_range(elem->n_sides()))
        if (!elem->neighbor_ptr(s))
        {
          auto points = sorted_point_tuple(*elem, s);
          auto it = mesh_faces.find(points);

          // I'd love to assert that we don't have any missing
          // matches, but our holes might themselves have holes
          if (it != mesh_faces.end())
          {
            auto [main_elem, main_side] = it->second.first;
            if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
            {
              hole_boundary_info.add_side(elem, s, new_hole_bcid);
              mesh_boundary_info.add_side(main_elem, main_side, inner_bcid);
            }
            // We would like to take this opportunity to identify the hole boundary
            // We set the boundary id to hole_i + 1 at this stage (the default)
            mesh_boundary_info.add_side(main_elem, main_side, hole_i + 1);
            it->second.second = true;
          }
        }
  }

  // Any sides that do not match the hole surfaces belong to the external boundary
  // We set the boundary id to 0 at this stage (the default)
  for (auto & [points, elem_side] : mesh_faces)
    if (!elem_side.second)
    {
      auto [main_elem, main_side] = elem_side.first;
      mesh_boundary_info.add_side(main_elem, main_side, 0);
    }

  // Now hole boundary ids are 1,2,..., N
  // Now external boundary id is 0
  // We will find the upper bound of the boundary ids of the mesh so that we would not overwrite
  // things during renumbering (because of the bcids used for stitching, this is different from
  // end_bcid)
  const auto temp_bcid_shift = MooseMeshUtils::getNextFreeBoundaryID(*mesh);
  // If we stitch holes, we want to make sure the netgen mesh has no conflicting boundary ids
  // with the hole meshes.
  // If we assign custom boundary ids, we want to make sure no overwriting occurs.
  // So we shift all these boundary ids by temp_bcid_shift + free_boundary_id
  // before we assign the new boundary ids
  if (_stitch_holes.size() || output_boundary_id.size())
    libMesh::MeshTools::Modification::change_boundary_id(
        *mesh, 0, temp_bcid_shift + free_boundary_id);
  if (_stitch_holes.size() || hole_boundary_ids.size())
    for (auto hole_i : index_range(_hole_ptrs))
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, hole_i + 1, hole_i + 1 + temp_bcid_shift + free_boundary_id);

  // Now we can reassign the boundary ids
  // If these ids are specified, we will use them
  // Otherwise, we will use the default ones
  if (output_boundary_id.size())
    libMesh::MeshTools::Modification::change_boundary_id(
        *mesh, temp_bcid_shift + free_boundary_id, output_boundary_id[0]);
  else
    libMesh::MeshTools::Modification::change_boundary_id(
        *mesh, temp_bcid_shift + free_boundary_id, free_boundary_id);

  if (hole_boundary_ids.size())
    for (auto hole_i : index_range(_hole_ptrs))
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, hole_i + 1 + temp_bcid_shift + free_boundary_id, hole_boundary_ids[hole_i]);
  else
    for (auto hole_i : index_range(_hole_ptrs))
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, hole_i + 1 + temp_bcid_shift + free_boundary_id, hole_i + 1 + free_boundary_id);

  for (auto hole_i : index_range(_hole_ptrs))
  {
    UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(**_hole_ptrs[hole_i]);

    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
    {
      // Retrieve subdomain name map from the mesh to be stitched and insert it into the main
      // subdomain map
      const auto & increment_subdomain_map = hole_mesh.get_subdomain_name_map();
      main_subdomain_map.insert(increment_subdomain_map.begin(), increment_subdomain_map.end());

      std::size_t n_nodes_stitched = mesh->stitch_meshes(hole_mesh,
                                                         inner_bcid,
                                                         new_hole_bcid,
                                                         TOLERANCE,
                                                         /*clear_stitched_bcids*/ true,
                                                         _verbose_stitching,
                                                         use_binary_search);

      if (!n_nodes_stitched)
        mooseError("Failed to stitch hole mesh ", hole_i, " to new tetrahedralization.");
    }
  }
  // Boundary names
  if (hole_boundary_ids.size())
    for (auto h : index_range(_hole_ptrs))
      mesh->get_boundary_info().sideset_name(hole_boundary_ids[h]) = hole_boundaries[h];
  if (output_boundary_id.size())
    mesh->get_boundary_info().sideset_name(output_boundary_id[0]) = output_boundary;

  // Check if one SubdomainName is shared by more than one subdomain ids
  std::set<SubdomainName> main_subdomain_map_name_list;
  for (auto const & id_name_pair : main_subdomain_map)
    main_subdomain_map_name_list.emplace(id_name_pair.second);
  if (main_subdomain_map.size() != main_subdomain_map_name_list.size())
    paramError("holes", "The hole meshes contain subdomain name maps with conflicts.");

  // We're done with the hole meshes now, and MeshGenerator doesn't
  // want them anymore either.
  for (auto & hole_ptr : _hole_ptrs)
    hole_ptr->reset();

  mesh->set_isnt_prepared();
  return mesh;
#else
  mooseError("Cannot use XYZDelaunayGenerator without NetGen-enabled libMesh.");
  return std::unique_ptr<MeshBase>();
#endif
}
