//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
      "boundary", "The input MeshGenerator defining the output outer boundary");

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
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");

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

  // Get ready to triangulate its boundary
  libMesh::NetGenMeshInterface ngint(*mesh);

  ngint.smooth_after_generating() = _smooth_tri;

  ngint.desired_volume() = _desired_volume;

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

  // The new triangulation by default assigns BCID i+1 to hole i ...
  // but we can't even use this for mesh stitching, because we can't
  // be sure it isn't also already in use on the hole's mesh and so we
  // won't be able to safely clear it afterwards.
  const boundary_id_type end_bcid = _hole_ptrs.size() + 1;

  // For the hole meshes that need to be stitched, we would like to make sure the hole boundary ids
  // and output boundary id are not conflicting with the existing boundary ids of the hole meshes to
  // be stitched.
  BoundaryID free_boundary_id = 0;
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
    for (auto h : index_range(_hole_ptrs))
      libMesh::MeshTools::Modification::change_boundary_id(*mesh, h + 1, h + 1 + free_boundary_id);
  }
  boundary_id_type new_hole_bcid = end_bcid + free_boundary_id;

  // We might be overriding the default bcid numbers.  We have to be
  // careful about how we renumber, though.  We pick unused temporary
  // numbers because e.g. "0->2, 2->0" is impossible to do
  // sequentially, but "0->N, 2->N+2, N->2, N+2->0" works.
  libMesh::MeshTools::Modification::change_boundary_id(
      *mesh, 0, (isParamValid("output_boundary") ? end_bcid : 0) + free_boundary_id);

  if (isParamValid("hole_boundaries"))
  {
    auto hole_boundaries = getParam<std::vector<BoundaryName>>("hole_boundaries");
    auto hole_boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, hole_boundaries, true);

    for (auto h : index_range(_hole_ptrs))
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, h + 1 + free_boundary_id, h + 1 + free_boundary_id + end_bcid);

    for (auto h : index_range(_hole_ptrs))
    {
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, h + 1 + free_boundary_id + end_bcid, hole_boundary_ids[h]);
      mesh->get_boundary_info().sideset_name(hole_boundary_ids[h]) = hole_boundaries[h];
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(hole_boundary_ids[h] + 1));
    }
  }

  if (isParamValid("output_boundary"))
  {
    const BoundaryName output_boundary = getParam<BoundaryName>("output_boundary");
    const std::vector<BoundaryID> output_boundary_id =
        MooseMeshUtils::getBoundaryIDs(*mesh, {output_boundary}, true);

    libMesh::MeshTools::Modification::change_boundary_id(
        *mesh, end_bcid + free_boundary_id, output_boundary_id[0]);
    mesh->get_boundary_info().sideset_name(output_boundary_id[0]) = output_boundary;

    new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(output_boundary_id[0] + 1));
  }

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
  std::unordered_map<std::tuple<Point, Point, Point>, std::pair<Elem *, unsigned int>> mesh_faces;

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
          mesh_faces.emplace(points, std::make_pair(elem, s));
        }

  auto & mesh_boundary_info = mesh->get_boundary_info();

  // Define a reference map variable for subdomain map
  auto & main_subdomain_map = mesh->set_subdomain_name_map();
  for (auto hole_i : index_range(_hole_ptrs))
  {
    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
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
              auto [main_elem, main_side] = it->second;
              hole_boundary_info.add_side(elem, s, new_hole_bcid);
              mesh_boundary_info.add_side(main_elem, main_side, inner_bcid);
            }
          }

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
