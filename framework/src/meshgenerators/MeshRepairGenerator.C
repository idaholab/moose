//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshRepairGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "GeometryUtils.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/boundary_info.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_c0polygon.h"
#include "libmesh/cell_c0polyhedron.h"
#include "libmesh/cell_polyhedron.h"

#include <array>
#include <limits>

registerMooseObject("MooseApp", MeshRepairGenerator);

InputParameters
MeshRepairGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Mesh generator to perform various improvement / fixing operations on an input mesh");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Name of the mesh generator providing the mesh");

  params.addParam<bool>("fix_node_overlap", false, "Whether to merge overlapping nodes");
  params.addParam<Real>(
      "node_overlap_tol", 1e-8, "Absolute tolerance for merging overlapping nodes");

  params.addParam<bool>(
      "fix_elements_orientation", false, "Whether to flip elements with negative volumes");

  params.addParam<bool>("separate_blocks_by_element_types",
                        false,
                        "Create new blocks if multiple element types are present in a block");

  params.addParam<bool>("merge_boundary_ids_with_same_name",
                        false,
                        "Merge boundaries if they have the same name but different boundary IDs");

  params.addParam<bool>(
      "fix_sliver_elements",
      false,
      "Whether to repair sliver (near-degenerate) first-order elements. A 2D sliver (TRI3, QUAD4, "
      "polygon) is absorbed into its longest-edge neighbor: a triangle sliver against a triangle "
      "neighbor splits that neighbor into two triangles (the mesh stays all-triangle), otherwise "
      "the neighbor absorbs the sliver's vertices and is promoted to a quadrilateral or polygon. A "
      "TET4 sliver is removed by edge collapse, keeping a valid all-tetrahedral conformal mesh. A "
      "flat PYRAMID5 sliver is absorbed into the element across its quad base, which becomes a "
      "polyhedron. Each repair keeps the mesh conformal, or leaves the sliver in place if no valid "
      "repair exists.");
  params.addRangeCheckedParam<Real>(
      "sliver_element_area_fraction",
      1e-10,
      "sliver_element_area_fraction>=0",
      "A 2D element whose area is below this fraction of the mesh surface-area scale is treated as "
      "a sliver (set to 0 to disable this test). Only used when 'fix_sliver_elements' is set.");
  params.addRangeCheckedParam<Real>(
      "sliver_element_flap_tol",
      0.02,
      "sliver_element_flap_tol>=0",
      "A 2D element is treated as a sliver if every vertex other than the two ends of its longest "
      "edge lies within this fraction of the longest-edge length from that edge, projecting onto "
      "its interior (set to 0 to disable this test). Only used when 'fix_sliver_elements' is "
      "set. In 3D this is the distance from the apex to its opposite face (the largest face for a "
      "TET4, the quad base for a PYRAMID5) as a fraction of sqrt(that face's area).");
  params.addRangeCheckedParam<Real>(
      "sliver_element_volume_fraction",
      1e-10,
      "sliver_element_volume_fraction>=0",
      "A TET4 or PYRAMID5 whose volume is below this fraction of the mesh bounding-box volume is "
      "treated as a sliver (set to 0 to disable this test). Only used when 'fix_sliver_elements' "
      "is "
      "enabled.");
  params.addRangeCheckedParam<Real>(
      "tet_collapse_volume_floor",
      1e-9,
      "tet_collapse_volume_floor>=0",
      "When repairing a TET4 sliver by edge collapse, a reshaped neighbor tet is rejected (the "
      "collapse is not performed) if its volume would drop below this fraction of the mesh "
      "bounding-box volume, to avoid inverting elements or creating new slivers.");

  params.addParam<bool>(
      "renumber_contiguously",
      false,
      "Whether to renumber the elements of the mesh so the numbering is contiguous");

  params.addParam<bool>(
      "split_nonconvex_polygons", false, "Split non-convex polygons to form convex ones");

  return params;
}

MeshRepairGenerator::MeshRepairGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _fix_overlapping_nodes(getParam<bool>("fix_node_overlap")),
    _node_overlap_tol(getParam<Real>("node_overlap_tol")),
    _fix_element_orientation(getParam<bool>("fix_elements_orientation")),
    _elem_type_separation(getParam<bool>("separate_blocks_by_element_types")),
    _boundary_id_merge(getParam<bool>("merge_boundary_ids_with_same_name")),
    _split_nonconvex_polygons(getParam<bool>("split_nonconvex_polygons")),
    _fix_sliver_elements(getParam<bool>("fix_sliver_elements")),
    _sliver_area_tol(getParam<Real>("sliver_element_area_fraction")),
    _sliver_flap_tol(getParam<Real>("sliver_element_flap_tol")),
    _sliver_volume_tol(getParam<Real>("sliver_element_volume_fraction")),
    _tet_collapse_volume_floor(getParam<Real>("tet_collapse_volume_floor"))
{
  if (!_fix_overlapping_nodes && !_fix_element_orientation && !_elem_type_separation &&
      !_boundary_id_merge && !getParam<bool>("renumber_contiguously") && !_split_nonconvex_polygons)

  if (!_fix_overlapping_nodes && !_fix_element_orientation && !_elem_type_separation &&
      !_boundary_id_merge && !_fix_sliver_elements && !getParam<bool>("renumber_contiguously"))
    mooseError("No specific item to fix. Are any of the parameters misspelled?");
}

std::unique_ptr<MeshBase>
MeshRepairGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // We're trying to repair a potentially broken mesh; we'll just
  // start with a full prepare rather than trying to be efficient and
  // risking missing something.
  mesh->prepare_for_use();

  // Blanket ban on distributed. This can be relaxed for some operations if needed
  if (!mesh->is_serial())
    mooseError("MeshRepairGenerator requires a serial mesh. The mesh should not be distributed.");

  if (_fix_overlapping_nodes)
    fixOverlappingNodes(mesh);

  // Repair sliver elements by absorbing them into their longest-edge neighbor
  if (_fix_sliver_elements)
    repairSlivers(mesh);

  // Repair sliver TET4 elements by edge collapse
  if (_fix_sliver_elements)
    repairTetSlivers(mesh);

  // Repair sliver PYRAMID5 elements by absorbing them into their quad-base neighbor
  if (_fix_sliver_elements)
    repairPyramidSlivers(mesh);

  // Flip orientation of elements to keep positive volumes
  if (_fix_element_orientation)
    MeshTools::Modification::orient_elements(*mesh);

  // Disambiguate any block that has elements of multiple types
  if (_elem_type_separation)
    separateSubdomainsByElementType(mesh);

  // Assign a single boundary ID to boundaries that have the same name
  if (_boundary_id_merge)
    MooseMeshUtils::mergeBoundaryIDsWithSameName(*mesh);

  // Renumber the mesh despite any mesh flag
  if (getParam<bool>("renumber_contiguously"))
  {
    const auto prev_status = mesh->allow_renumbering();
    mesh->allow_renumbering(true);
    mesh->renumber_nodes_and_elements();
    mesh->allow_renumbering(prev_status);
  }

  // Split non-convex polygons to make convex ones
  if (_split_nonconvex_polygons)
    splitNonConvexPolygons(mesh);

  mesh->unset_is_prepared();
  return mesh;
}

void
MeshRepairGenerator::fixOverlappingNodes(std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_fixed_nodes = 0;
  auto pl = mesh->sub_point_locator();
  pl->set_close_to_point_tol(_node_overlap_tol);

  std::unordered_set<dof_id_type> nodes_removed;
  // loop on nodes
  for (auto & node : mesh->node_ptr_range())
  {
    // this node has already been removed
    if (nodes_removed.count(node->id()))
      continue;

    // find all the elements around this node
    std::set<const Elem *> elements;
    (*pl)(*node, elements);

    for (auto & elem : elements)
    {
      bool found = false;
      for (auto & elem_node : elem->node_ref_range())
      {
        if (node->id() == elem_node.id())
        {
          found = true;
          break;
        }
      }
      if (!found)
      {
        for (auto & elem_node : elem->node_ref_range())
        {
          if (elem_node.id() == node->id())
            continue;
          const Real tol = _node_overlap_tol;
          // Compares the coordinates
          const auto x_node = (*node)(0);
          const auto x_elem_node = elem_node(0);
          const auto y_node = (*node)(1);
          const auto y_elem_node = elem_node(1);
          const auto z_node = (*node)(2);
          const auto z_elem_node = elem_node(2);

          if (MooseUtils::absoluteFuzzyEqual(x_node, x_elem_node, tol) &&
              MooseUtils::absoluteFuzzyEqual(y_node, y_elem_node, tol) &&
              MooseUtils::absoluteFuzzyEqual(z_node, z_elem_node, tol))
          {
            // Merging two nodes from the same element is almost never a good idea
            if (elem->get_node_index(node) != libMesh::invalid_uint)
            {
              _console << "Two overlapping nodes in element " << elem->id() << " right by "
                       << elem->vertex_average() << ".\n They will not be stitched" << std::endl;
              continue;
            }

            // Coordinates are the same but it's not the same node
            // Replace the node in the element
            const_cast<Elem *>(elem)->set_node(elem->get_node_index(&elem_node), node);
            nodes_removed.insert(elem_node.id());

            num_fixed_nodes++;
            if (num_fixed_nodes < 10)
              _console << "Stitching nodes " << *node << " and            " << elem_node
                       << std::endl;
            else if (num_fixed_nodes == 10)
              _console << "Node stitching will now proceed silently." << std::endl;
          }
        }
      }
    }
  }
  _console << "Number of overlapping nodes which got merged: " << num_fixed_nodes << std::endl;
  if (mesh->allow_renumbering())
    mesh->renumber_nodes_and_elements();
  else
  {
    mesh->remove_orphaned_nodes();
    mesh->update_parallel_id_counts();
  }
}

void
MeshRepairGenerator::separateSubdomainsByElementType(std::unique_ptr<MeshBase> & mesh) const
{
  std::set<subdomain_id_type> ids;
  mesh->subdomain_ids(ids);
  // loop on sub-domain
  for (const auto id : ids)
  {
    // Gather all the element types and blocks
    // ElemType defines an enum for geometric element types
    std::set<ElemType> types;
    // loop on elements within this sub-domain
    for (auto & elem : mesh->active_subdomain_elements_ptr_range(id))
      types.insert(elem->type());

    // This call must be performed on all processes
    auto next_block_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

    if (types.size() > 1)
    {
      subdomain_id_type i = 0;
      for (const auto type : types)
      {
        auto new_id = next_block_id + i++;
        // Create blocks when a block has multiple element types
        mesh->subdomain_name(new_id) = mesh->subdomain_name(id) + "_" + Moose::stringify(type);

        // Re-assign elements to the new blocks
        for (auto elem : mesh->active_subdomain_elements_ptr_range(id))
          if (elem->type() == type)
            elem->subdomain_id() = new_id;
      }
    }
  }
}

void
MeshRepairGenerator::splitNonConvexPolygons(std::unique_ptr<MeshBase> & mesh) const
{
  // Counters to keep track of what happened in the splitting
  unsigned int num_polygons = 0;
  unsigned int num_nonconvex = 0;
  unsigned int num_triangulated = 0;
  std::vector<Elem *> elems_to_delete;
  std::vector<std::unique_ptr<libMesh::C0Polygon>> elements_to_add_to_mesh;

  // Missing parallel packing for polys
  if (!mesh->is_serial())
    mooseError("MeshRepairGenerator requires a serial mesh for this operation. "
               "The mesh should not be distributed.");

  for (auto elem : mesh->element_ptr_range())
  {
    // We are not splitting non-convex quads. Maybe later.
    if (elem->type() == libMesh::C0POLYGON)
    {
      num_polygons++;

      // Lambda function to determine if an element is convex
      auto is_convex = [](const Elem * elem,
                          std::vector<std::pair<int, Real>> & obtuse_angle_nodes) -> bool
      {
        mooseAssert(elem, "Should have an elem");
        mooseAssert(obtuse_angle_nodes.empty(), "Should not be empty");

        // Find the normal to the element plane
        const auto elem_centroid = elem->vertex_average();
        const auto n_nodes = elem->n_nodes();
        Point plane_normal;
        // Two nodes could be aligned with the centroid in a non-convex polygon
        for (const auto i : make_range(n_nodes))
        {
          const auto v1 = elem_centroid - elem->point(i);
          const auto v2 = elem_centroid - elem->point((i + 1) % n_nodes);
          if (!MooseUtils::absoluteFuzzyEqual((v1.cross(v2)).norm_sq(), 0))
          {
            plane_normal = v1.cross(v2).unit();
            break;
          }
        }

        // Look for angles > 180 deg with the cross product
        bool convex = true;
        for (const auto i : make_range(n_nodes))
        {
          const auto n1 = elem->point(i);
          const auto n2 = elem->point((i + 1) % n_nodes);
          const auto n3 = elem->point((i + 2) % n_nodes);
          const auto top_dir = (n2 - n1).cross(n3 - n2);

          if (plane_normal * top_dir <= 0)
          {
            convex = false;
            if (!MooseUtils::absoluteFuzzyEqual(plane_normal * top_dir, 0))
              obtuse_angle_nodes.push_back(
                  std::make_pair<int, Real>(i, plane_normal * top_dir / top_dir.norm()));
            // n1, n2 and n3 are likely aligned
            else
              obtuse_angle_nodes.push_back(std::make_pair<int, Real>(i, -1));
          }
        }
        return convex;
      };

      std::vector<std::pair<int, Real>> parent_obtuse_angles;
      const auto convex = is_convex(elem, parent_obtuse_angles);

      if (!convex)
        num_nonconvex++;
      // Move to next one if it is convex, no need to fix it
      else
        continue;

      // Lambda function to split a polygon into two polygons at the specified nodes
      // TODO: we could try to split a polygon into more than two polygons
      auto cut_polygon = [&mesh](const Elem * elem, unsigned int node_i_cut1, unsigned node_i_cut2)
          -> std::pair<std::unique_ptr<libMesh::C0Polygon>, std::unique_ptr<libMesh::C0Polygon>>
      {
        const auto cut1 = std::min(node_i_cut1, node_i_cut2);
        const auto cut2 = std::max(node_i_cut1, node_i_cut2);

        // Count the number of sides on each side of the cut
        const auto ns1 = cut2 - cut1 + 1;
        const auto ns2 = elem->n_nodes() - ns1 + 2;
        mooseAssert(ns1 >= 3, "Should cut at least a triangle");
        mooseAssert(ns2 >= 3, "Should cut at least a triangle");

        // Create the children and add their nodes
        auto child1 = std::make_unique<libMesh::C0Polygon>(ns1);
        for (const auto i_n : make_range(cut1, cut2 + 1))
          child1->set_node(i_n - cut1, mesh->node_ptr(elem->node_ptr(i_n)->id()));

        auto child2 = std::make_unique<libMesh::C0Polygon>(ns2);
        for (const auto i_n : make_range(cut2, elem->n_nodes() + cut1 + 1))
          child2->set_node((i_n - cut2) % child2->n_nodes(),
                           mesh->node_ptr(elem->node_ptr(i_n % elem->n_nodes())->id()));

        return std::make_pair(std::move(child1), std::move(child2));
      };

      // If not convex, try to split it.
      // Let's first try to split it recursively at every large angle
      // NOTE: These vectors of unique pointer keep the memory ownership of the elements
      // during the loop attempting to fix the polygon with the heuristic below
      bool failed = false;
      std::vector<std::unique_ptr<libMesh::C0Polygon>> elements_to_add_to_mesh_temp;
      std::vector<std::pair<std::unique_ptr<Elem>, std::vector<std::pair<int, Real>>>> elems_to_cut;

      // Create a clone of the first element to simplify logic
      std::unique_ptr<libMesh::C0Polygon> base_elem =
          std::make_unique<libMesh::C0Polygon>(elem->n_nodes());
      for (const auto i_n : make_range(elem->n_nodes()))
        base_elem->set_node(i_n, elem->node_ptr(i_n));
      elems_to_cut.push_back(std::make_pair(std::move(base_elem), parent_obtuse_angles));

      while (!failed && !elems_to_cut.empty())
      {
        auto & [current_elem, large_angles] = elems_to_cut.back();

        // Split the element on one side at the node with the worst obtuse angle
        // TODO: try all of them instead to see if a single cut can fix the element
        auto worst_angle_it =
            std::min_element(large_angles.begin(),
                             large_angles.end(),
                             [](std::pair<int, Real> lhs, std::pair<int, Real> rhs) -> bool
                             { return lhs.second < rhs.second; });
        unsigned int worst_angle_i = (*worst_angle_it).first;
        const auto n_nodes = current_elem->n_nodes();

        // Split the element on the other side at roughly the opposite node
        // TODO: we could try all of the 'other' nodes here too to see if a single cut can fix the
        // element NOTE: if trying multiple cuts (not implemented), we should remove the opposite
        // from the large_angles
        const auto opposite = ((worst_angle_i + n_nodes / 2) % n_nodes);

        auto [child1, child2] = cut_polygon(current_elem.get(), worst_angle_i, opposite);

        // Check the two fragments
        std::vector<std::pair<int, Real>> child1_large_angles;
        bool is_convex1 = is_convex(child1.get(), child1_large_angles);
        std::vector<std::pair<int, Real>> child2_large_angles;
        bool is_convex2 = is_convex(child2.get(), child2_large_angles);

        // Can we keep going?
        if (child1->n_nodes() < 4 && !is_convex1)
          failed = true;
        if (child2->n_nodes() < 4 && !is_convex2)
          failed = true;

        // We managed to cut the elements into a convex part, but it's flipped
        // so the element is likely "outside" of the starting element
        if (is_convex1 && child1->volume() <= 0)
          failed = true;
        if (is_convex2 && child2->volume() <= 0)
          failed = true;

        // Current element is done
        // NOTE: if trying multiple cuts (not implemented), we should not erase yet, we should
        // instead only remove the cut we tried from the 'large_angles' vector used to pick cuts
        large_angles.erase(worst_angle_it);
        elems_to_cut.pop_back();

        // Add non convex elements to the list of elements to cut
        if (!is_convex1)
          elems_to_cut.push_back(std::make_pair(std::move(child1), child1_large_angles));
        if (!is_convex2)
          elems_to_cut.push_back(std::make_pair(std::move(child2), child2_large_angles));

        // Add convex elements to the mesh
        // NOTE: if trying multiple cuts, we should not do this yet
        if (is_convex1)
          elements_to_add_to_mesh_temp.push_back(std::move(child1));
        if (is_convex2)
          elements_to_add_to_mesh_temp.push_back(std::move(child2));
      }
      bool fixed_it = !failed;

      // Keep all the cut elements, to be added at the end to avoid invalidating iterators
      if (fixed_it)
        for (auto & elem_uptr : elements_to_add_to_mesh_temp)
        {
          elem_uptr->inherit_data_from(*elem);
          elements_to_add_to_mesh.push_back(std::move(elem_uptr));
        }

      // Heuristic did not work, just use a triangulation
      // NOTE: This is not the triangulation of the polygon. It is simple though
      if (!fixed_it)
      {
        const auto centroid_node = mesh->add_point(elem->true_centroid());
        num_triangulated++;
        const auto * poly = dynamic_cast<libMesh::C0Polygon *>(elem);
        const auto n_sides = poly->n_sides();
        for (const auto i_tr : make_range(n_sides))
        {
          auto new_elem = std::make_unique<libMesh::C0Polygon>(3);
          new_elem->set_node(0, mesh->node_ptr(elem->node_ptr(i_tr)->id()));
          new_elem->set_node(1, centroid_node);
          new_elem->set_node(2, mesh->node_ptr(elem->node_ptr((i_tr + 1) % n_sides)->id()));

          // Check for degenerate case
          if (MooseUtils::absoluteFuzzyEqual(
                  (*(Point *)centroid_node - *(Point *)elem->node_ptr(i_tr))
                      .cross(*(Point *)centroid_node -
                             *(Point *)elem->node_ptr((i_tr + 1) % n_sides))
                      .norm_sq(),
                  0))
            mooseError("Manual triangulation failed as two consecutive nodes are aligned with the "
                       "centroid");

          new_elem->inherit_data_from(*elem);
          elements_to_add_to_mesh.push_back(std::move(new_elem));
        }
      }
      // Element got fixed, can be deleted after (can't while using the range)
      elems_to_delete.push_back(elem);
    }
  }
  // Delete the original element
  for (auto elem : elems_to_delete)
    mesh->delete_elem(elem);
  // Add the new ones
  for (auto & new_elem_ptr : elements_to_add_to_mesh)
    mesh->add_elem(std::move(new_elem_ptr));

  _console << "Number of non-convex polygons which got split into convex polygons: "
           << num_nonconvex << std::endl;
  if (num_triangulated)
    _console << "Number of non-convex polygons split using a triangulation: " << num_triangulated
             << ", using heuristic: " << num_nonconvex - num_triangulated << std::endl;
  if (!num_polygons)
    mooseWarning("No C0 polygons in mesh: the polyon convexity fix did nothing");
}

void
MeshRepairGenerator::repairSlivers(std::unique_ptr<MeshBase> & mesh) const
{
  // Surface-area scale of the whole mesh, used by the area-based sliver test
  const auto bbox = MeshTools::create_bounding_box(*mesh);
  const Point ext = bbox.max() - bbox.min();
  const Real surface_scale =
      std::abs(ext(0) * ext(1)) + std::abs(ext(0) * ext(2)) + std::abs(ext(1) * ext(2));
  const Real area_thresh = std::max(surface_scale, Real(1e-30)) * _sliver_area_tol;

  // Index of the longest edge (between vertices lng and lng+1) of a 2D element
  auto longestEdge = [](const Elem & e)
  {
    const auto nv = e.n_vertices();
    unsigned int lng = 0;
    Real longest = -1;
    for (const auto i : make_range(nv))
    {
      const Real l = (e.point((i + 1) % nv) - e.point(i)).norm();
      if (l > longest)
      {
        longest = l;
        lng = i;
      }
    }
    return lng;
  };
  // Area of a (planar) 2D element, by fanning from its first vertex
  auto elemArea = [](const Elem & e)
  {
    const auto nv = e.n_vertices();
    Point area_vec;
    for (const auto i : make_range(1u, nv - 1))
      area_vec += (e.point(i) - e.point(0)).cross(e.point(i + 1) - e.point(0));
    return 0.5 * area_vec.norm();
  };
  // Whether vertex P lies within tol*|A-B| of segment A-B (distance to the clamped segment, so a
  // vertex sitting above an endpoint of a thin strip still counts)
  auto nearEdge = [](const Point & P, const Point & A, const Point & B, const Real tol)
  {
    const Point AB = B - A;
    const Real elen_sq = AB.norm_sq();
    if (elen_sq <= 0)
      return false;
    const Real t = std::max(Real(0), std::min(Real(1), ((P - A) * AB) / elen_sq));
    return (P - (A + t * AB)).norm() < tol * std::sqrt(elen_sq);
  };
  // A 2D element is a sliver if, against its longest edge, every other vertex is nearly on that
  // edge (flap test) or the element area is negligible (area test). Either test can be disabled by
  // setting its tolerance to 0.
  auto isSliver = [&](const Elem & e, const unsigned int lng)
  {
    const auto nv = e.n_vertices();
    const Point & A = e.point(lng);
    const Point & B = e.point((lng + 1) % nv);
    if ((B - A).norm() == 0)
      return false;
    if (_sliver_area_tol > 0 && elemArea(e) < area_thresh)
      return true;
    if (_sliver_flap_tol > 0)
    {
      for (const auto k : make_range(nv))
        if (k != lng && k != (lng + 1) % nv && !nearEdge(e.point(k), A, B, _sliver_flap_tol))
          return false;
      return true;
    }
    return false;
  };
  // First-order linear 2D element (TRI3, QUAD4, C0POLYGON): every node is a vertex
  auto isLinear2D = [](const Elem & e) { return e.dim() == 2 && e.n_nodes() == e.n_vertices(); };

  // Undirected edge (sorted node-id pair) key
  auto edge_key = [](dof_id_type a, dof_id_type b)
  { return std::make_pair(std::min(a, b), std::max(a, b)); };

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  std::size_t num_repaired = 0;

  // Repair in passes. Within a pass we only perform repairs whose sliver+neighbor node sets are
  // disjoint from the other repairs in that pass; node-disjoint implies edge-disjoint, so the edge
  // map built at the start of the pass stays valid for every repair we make. Slivers adjacent to a
  // repair are deferred to a later pass (rebuilt from the updated mesh). Repairs never create new
  // slivers, so this terminates.
  bool repaired_in_pass = true;
  while (repaired_in_pass)
  {
    repaired_in_pass = false;

    // Edge -> ids of the 2D elements using it, and the slivers, from the current mesh
    std::map<std::pair<dof_id_type, dof_id_type>, std::vector<dof_id_type>> edge_to_elems;
    std::vector<dof_id_type> sliver_ids;
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      if (!isLinear2D(*elem))
        continue;
      const auto nv = elem->n_vertices();
      for (const auto i : make_range(nv))
        edge_to_elems[edge_key(elem->node_id(i), elem->node_id((i + 1) % nv))].push_back(
            elem->id());
      if (isSliver(*elem, longestEdge(*elem)))
        sliver_ids.push_back(elem->id());
    }

    // Nodes already used by a repair this pass; a sliver or neighbor touching one is deferred
    std::unordered_set<dof_id_type> touched_nodes;
    auto touches_repaired = [&touched_nodes](const Elem & e)
    {
      for (const auto i : make_range(e.n_vertices()))
        if (touched_nodes.count(e.node_id(i)))
          return true;
      return false;
    };

    for (const auto sid : sliver_ids)
    {
      Elem * s = mesh->query_elem_ptr(sid);
      if (!s || touches_repaired(*s))
        continue;
      const auto nv = s->n_vertices();
      const unsigned int lng = longestEdge(*s);
      if (!isSliver(*s, lng))
        continue;

      Node * A = s->node_ptr(lng);
      Node * B = s->node_ptr((lng + 1) % nv);
      // The sliver's other vertices, in order from just after B around to just before A. They get
      // inserted into the neighbor's shared edge (going B -> chain -> A).
      std::vector<Node *> chain;
      for (const auto off : make_range(2u, nv))
        chain.push_back(s->node_ptr((lng + off) % nv));

      // Find a 2D neighbor sharing the longest edge A-B
      Elem * neighbor = nullptr;
      for (const auto nid : libmesh_map_find(edge_to_elems, edge_key(A->id(), B->id())))
      {
        if (nid == sid)
          continue;
        Elem * cand = mesh->query_elem_ptr(nid);
        if (cand && isLinear2D(*cand))
        {
          neighbor = cand;
          break;
        }
      }
      // No neighbor to absorb the sliver (boundary edge), or the neighbor is already part of a
      // repair this pass: leave the sliver for now
      if (!neighbor || touches_repaired(*neighbor))
        continue;

      // Local side of the neighbor that is the shared edge A-B
      const auto mv = neighbor->n_vertices();
      unsigned int eidx = libMesh::invalid_uint;
      for (const auto e : make_range(mv))
      {
        const auto a = neighbor->node_id(e), b = neighbor->node_id((e + 1) % mv);
        if ((a == A->id() && b == B->id()) || (a == B->id() && b == A->id()))
        {
          eidx = e;
          break;
        }
      }
      if (eidx == libMesh::invalid_uint)
        continue; // should not happen
      const subdomain_id_type sub = neighbor->subdomain_id();

      // Record the sliver and neighbor nodes so adjacent slivers are deferred to a later pass
      for (const auto i : make_range(nv))
        touched_nodes.insert(s->node_id(i));
      for (const auto e : make_range(mv))
        touched_nodes.insert(neighbor->node_id(e));

      // Capture the boundary ids on the sides of the sliver and the neighbor, keyed by node-id
      // pair, so we can put them back on the surviving edges
      std::map<std::pair<dof_id_type, dof_id_type>, std::vector<boundary_id_type>> edge_bcs;
      std::vector<boundary_id_type> side_ids;
      for (const Elem * t : {const_cast<const Elem *>(s), const_cast<const Elem *>(neighbor)})
        for (const auto e : make_range(t->n_vertices()))
        {
          boundary_info.boundary_ids(t, cast_int<unsigned short>(e), side_ids);
          if (!side_ids.empty())
          {
            auto & dst = edge_bcs[edge_key(t->node_id(e), t->node_id((e + 1) % t->n_vertices()))];
            dst.insert(dst.end(), side_ids.begin(), side_ids.end());
          }
        }
      boundary_info.remove(s);
      boundary_info.remove(neighbor);

      // The element(s) resulting from the repair, onto which we restore the boundary ids
      std::vector<Elem *> result_elems;
      if (nv == 3 && mv == 3)
      {
        // Triangle sliver against a triangle neighbor: split the neighbor into two triangles at the
        // sliver's apex, keeping the mesh all-triangle (no promotion to a quad)
        Node * apex = chain.front();
        Node * e1 = neighbor->node_ptr((eidx + 1) % 3);
        Node * w = neighbor->node_ptr((eidx + 2) % 3);
        neighbor->set_node((eidx + 1) % 3, apex); // reshape the neighbor into (e0, apex, w)
        auto half = std::make_unique<Tri3>();
        half->set_node(0, apex);
        half->set_node(1, e1);
        half->set_node(2, w);
        half->subdomain_id() = sub;
        result_elems.push_back(neighbor);
        result_elems.push_back(mesh->add_elem(std::move(half)));
        mesh->delete_elem(s);
      }
      else
      {
        // Otherwise promote the neighbor: insert the sliver's chain into its shared edge. If the
        // neighbor traverses the edge B->A, insert the chain as-is; if A->B, insert it reversed.
        const bool edge_is_BA = (neighbor->node_id(eidx) == B->id());
        std::vector<Node *> new_nodes;
        new_nodes.reserve(mv + chain.size());
        for (const auto e : make_range(mv))
        {
          new_nodes.push_back(neighbor->node_ptr(e));
          if (e == eidx)
          {
            if (edge_is_BA)
              new_nodes.insert(new_nodes.end(), chain.begin(), chain.end());
            else
              new_nodes.insert(new_nodes.end(), chain.rbegin(), chain.rend());
          }
        }
        // A quad if it now has 4 vertices, otherwise a polygon
        std::unique_ptr<Elem> promoted;
        if (new_nodes.size() == 4)
          promoted = std::make_unique<Quad4>();
        else
          promoted = std::make_unique<libMesh::C0Polygon>(new_nodes.size());
        for (const auto i : index_range(new_nodes))
          promoted->set_node(i, new_nodes[i]);
        promoted->subdomain_id() = sub;
        result_elems.push_back(mesh->add_elem(std::move(promoted)));
        mesh->delete_elem(s);
        mesh->delete_elem(neighbor);
      }

      // Restore the captured boundary ids onto the matching edges of the resulting element(s)
      for (Elem * t : result_elems)
        for (const auto e : make_range(t->n_vertices()))
        {
          auto it = edge_bcs.find(edge_key(t->node_id(e), t->node_id((e + 1) % t->n_vertices())));
          if (it != edge_bcs.end())
            for (const auto bid : it->second)
              boundary_info.add_side(t, cast_int<unsigned short>(e), bid);
        }

      ++num_repaired;
      repaired_in_pass = true;
    }
  }

  if (num_repaired)
  {
    mesh->prepare_for_use();
    _console << "Number of sliver elements repaired: " << num_repaired << std::endl;
  }
}

void
MeshRepairGenerator::repairTetSlivers(std::unique_ptr<MeshBase> & mesh) const
{
  // Bounding-box volume scale (3D analog of the 2D surface_scale) and derived thresholds
  const auto bbox = MeshTools::create_bounding_box(*mesh);
  const Point ext = bbox.max() - bbox.min();
  const Real vol_scale = std::max(std::abs(ext(0) * ext(1) * ext(2)), Real(1e-30));
  const Real vol_thresh = vol_scale * _sliver_volume_tol;
  // A reshaped tet must keep |volume| above this floor (10x to avoid creating a new sliver)
  const Real invert_floor = 10.0 * vol_scale * _tet_collapse_volume_floor;

  // Signed volume of a tet from its four corner points
  auto tetVol = [](const Point & a, const Point & b, const Point & c, const Point & d)
  { return (b - a) * (c - a).cross(d - a) / 6.0; };

  // Signed volume of TET4 e with node g replaced by node k (no mutation of the mesh)
  auto tetVolSub = [&](const Elem & e, const Node * g, const Node * k)
  {
    std::array<const Point *, 4> p;
    for (const auto i : make_range(4u))
      p[i] = (e.node_ptr(i) == g) ? static_cast<const Point *>(k)
                                  : static_cast<const Point *>(e.node_ptr(i));
    return tetVol(*p[0], *p[1], *p[2], *p[3]);
  };

  // Largest face of a TET4: returns (area, side index) and sets apex_local to the off-face vertex
  auto largestFace = [](const Elem & e, unsigned int & apex_local)
  {
    Real best = -1;
    unsigned int best_side = 0;
    for (const auto s : make_range(e.n_sides()))
    {
      const auto ns = e.nodes_on_side(s);
      const Real a =
          0.5 * (e.point(ns[1]) - e.point(ns[0])).cross(e.point(ns[2]) - e.point(ns[0])).norm();
      if (a > best)
      {
        best = a;
        best_side = s;
      }
    }
    const auto ns = e.nodes_on_side(best_side);
    apex_local = 0;
    for (const auto i : make_range(4u))
      if (i != ns[0] && i != ns[1] && i != ns[2])
        apex_local = i;
    return std::make_pair(best, best_side);
  };

  // A TET4 is a sliver if its volume is negligible or its apex is flat against its largest face
  auto isTetSliver = [&](const Elem & e)
  {
    if (e.type() != TET4)
      return false;
    if (_sliver_volume_tol > 0 && std::abs(e.volume()) < vol_thresh)
      return true;
    if (_sliver_flap_tol > 0)
    {
      unsigned int apex_local;
      const auto [area, side] = largestFace(e, apex_local);
      if (area > 0)
      {
        const auto ns = e.nodes_on_side(side);
        const Real d = std::sqrt(geom_utils::pointTriangleDistanceSq(
            e.point(apex_local), e.point(ns[0]), e.point(ns[1]), e.point(ns[2])));
        if (d < _sliver_flap_tol * std::sqrt(area))
          return true;
      }
    }
    return false;
  };

  // Sorted node-id key of a tet face (for boundary-id transfer and the manifold check)
  auto faceKey = [](dof_id_type a, dof_id_type b, dof_id_type c)
  {
    std::array<dof_id_type, 3> f{a, b, c};
    std::sort(f.begin(), f.end());
    return f;
  };

  std::size_t num_repaired = 0;
  std::size_t num_skipped = 0;

  bool repaired_in_pass = true;
  while (repaired_in_pass)
  {
    repaired_in_pass = false;

    // node id -> ids of ALL incident elements (used for the collapse star and the non-TET4 guard);
    // a tet-face -> use count (to find boundary faces independently of possibly-stale neighbor
    // links); and the current TET4 slivers
    std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_elems;
    std::map<std::array<dof_id_type, 3>, unsigned int> face_count;
    std::vector<dof_id_type> sliver_ids;
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      for (const auto n : make_range(elem->n_nodes()))
        node_to_elems[elem->node_id(n)].push_back(elem->id());
      if (elem->type() == TET4)
      {
        for (const auto s : make_range(elem->n_sides()))
        {
          const auto ns = elem->nodes_on_side(s);
          ++face_count[faceKey(elem->node_id(ns[0]), elem->node_id(ns[1]), elem->node_id(ns[2]))];
        }
        if (isTetSliver(*elem))
          sliver_ids.push_back(elem->id());
      }
    }
    // A node on a face used by a single tet is on the mesh boundary
    std::unordered_set<dof_id_type> boundary_nodes;
    for (const auto & [f, count] : face_count)
      if (count == 1)
        boundary_nodes.insert(f.begin(), f.end());

    BoundaryInfo & boundary_info = mesh->get_boundary_info();
    std::unordered_set<dof_id_type> touched_nodes;
    auto touches_repaired = [&touched_nodes](const Elem & e)
    {
      for (const auto i : make_range(e.n_nodes()))
        if (touched_nodes.count(e.node_id(i)))
          return true;
      return false;
    };

    for (const auto sid : sliver_ids)
    {
      Elem * s = mesh->query_elem_ptr(sid);
      if (!s || s->type() != TET4 || touches_repaired(*s) || !isTetSliver(*s))
        continue;

      // Evaluate a candidate "collapse gone-node G onto kept-node K": returns whether it is a
      // valid, boundary-safe, non-inverting, manifold collapse, and (if so) its deleted/reshaped
      // tets and a quality score (the minimum reshaped-tet relative volume).
      Node * best_K = nullptr;
      Node * best_G = nullptr;
      std::vector<dof_id_type> best_deleted, best_reshaped;
      Real best_score = -1;
      Real best_edge_len = std::numeric_limits<Real>::max();

      auto consider = [&](Node * K, Node * G, Real edge_len)
      {
        // Boundary rules: never move/remove a boundary node onto an interior position, and (to keep
        // the boundary undistorted) never collapse an edge whose both endpoints are on the boundary
        const bool K_bnd = boundary_nodes.count(K->id());
        const bool G_bnd = boundary_nodes.count(G->id());
        if (G_bnd && !K_bnd)
          return; // would delete a boundary node
        if (G_bnd && K_bnd)
          return; // conservative: do not modify the boundary surface

        // The collapse star: all elements incident to G or K
        std::set<dof_id_type> star;
        for (const auto eid : libmesh_map_find(node_to_elems, G->id()))
          star.insert(eid);
        for (const auto eid : libmesh_map_find(node_to_elems, K->id()))
          star.insert(eid);

        // Non-TET4 guard: a hybrid star cannot be reshaped by tet collapse
        for (const auto eid : star)
        {
          const Elem * e = mesh->query_elem_ptr(eid);
          if (!e || e->type() != TET4)
            return;
        }

        // Classify the tets incident to G: deleted (also contain K) vs reshaped (do not)
        std::vector<dof_id_type> deleted, reshaped;
        for (const auto eid : libmesh_map_find(node_to_elems, G->id()))
        {
          const Elem * e = mesh->query_elem_ptr(eid);
          bool has_K = false;
          for (const auto i : make_range(4u))
            if (e->node_id(i) == K->id())
              has_K = true;
          (has_K ? deleted : reshaped).push_back(eid);
        }

        // Invertibility: every reshaped tet must keep its orientation and stay non-degenerate
        Real score = std::numeric_limits<Real>::max();
        for (const auto eid : reshaped)
        {
          const Elem * e = mesh->query_elem_ptr(eid);
          const Real vb = e->volume();
          const Real va = tetVolSub(*e, G, K);
          if (std::abs(va) < invert_floor)
            return; // would invert or create a new sliver
          if (std::abs(vb) >= invert_floor)
          {
            if ((vb > 0) != (va > 0))
              return; // orientation flipped
          }
          else if (va <= 0)
            return; // reshaping a near-degenerate tet: require a positive result
          score = std::min(score, std::abs(va) / vol_scale);
        }

        // Validity: build the surviving tets (deleted removed, G -> K applied) and check that the
        // result stays a valid manifold mesh with an unchanged boundary:
        //  (a) no survivor is degenerate (repeated node) and no two survivors coincide (a fold);
        //  (b) no triangular face is shared by more than two survivors (non-manifold);
        //  (c) every external face of a deleted tet is still carried by a survivor (no boundary
        //  hole).
        std::set<dof_id_type> deleted_set(deleted.begin(), deleted.end());
        std::map<std::array<dof_id_type, 3>, unsigned int> survivor_face_count;
        std::set<std::array<dof_id_type, 4>> survivor_keys;
        for (const auto eid : star)
        {
          if (deleted_set.count(eid))
            continue;
          const Elem * e = mesh->query_elem_ptr(eid);
          std::array<dof_id_type, 4> nk;
          for (const auto i : make_range(4u))
            nk[i] = (e->node_id(i) == G->id()) ? K->id() : e->node_id(i);
          auto sorted_nk = nk;
          std::sort(sorted_nk.begin(), sorted_nk.end());
          if (sorted_nk[0] == sorted_nk[1] || sorted_nk[1] == sorted_nk[2] ||
              sorted_nk[2] == sorted_nk[3])
            return; // degenerate (repeated node) survivor
          if (!survivor_keys.insert(sorted_nk).second)
            return; // duplicate tet -> fold
          for (const auto i : make_range(4u))
            for (const auto j : make_range(i + 1, 4u))
              for (const auto l : make_range(j + 1, 4u))
                ++survivor_face_count[faceKey(nk[i], nk[j], nk[l])];
        }
        for (const auto & [f, count] : survivor_face_count)
          if (count > 2)
            return; // non-manifold face

        auto sub = [&](dof_id_type n) { return n == G->id() ? K->id() : n; };
        for (const auto eid : deleted)
        {
          const Elem * e = mesh->query_elem_ptr(eid);
          for (const auto sd : make_range(e->n_sides()))
          {
            const auto ns = e->nodes_on_side(sd);
            auto fc =
                face_count.find(faceKey(e->node_id(ns[0]), e->node_id(ns[1]), e->node_id(ns[2])));
            if (fc == face_count.end() || fc->second != 1)
              continue; // interior face: fine to dissolve
            const auto fa = sub(e->node_id(ns[0])), fb = sub(e->node_id(ns[1])),
                       fcc = sub(e->node_id(ns[2]));
            if (fa == fb || fb == fcc || fa == fcc)
              continue; // face sat on the collapsed edge: it vanishes by design
            if (!survivor_face_count.count(faceKey(fa, fb, fcc)))
              return; // an external face would disappear -> a boundary hole
          }
        }

        // Feasible: keep the best by quality, then shortest collapsed edge, then deterministic id
        if (score > best_score || (score == best_score && edge_len < best_edge_len) ||
            (score == best_score && edge_len == best_edge_len &&
             K->id() < (best_K ? best_K->id() : 0)))
        {
          best_score = score;
          best_edge_len = edge_len;
          best_K = K;
          best_G = G;
          best_deleted = deleted;
          best_reshaped = reshaped;
        }
      };

      // Try all 6 edges of the sliver, both directions
      static const unsigned int tet_edges[6][2] = {{0, 1}, {1, 2}, {0, 2}, {0, 3}, {1, 3}, {2, 3}};
      for (const auto & ed : tet_edges)
      {
        Node * a = s->node_ptr(ed[0]);
        Node * b = s->node_ptr(ed[1]);
        const Real len = (*a - *b).norm();
        consider(a, b, len);
        consider(b, a, len);
      }

      if (!best_K)
        continue; // no valid collapse this pass; remaining slivers are counted at the end

      // Whole-star node-disjointness: defer if any node of the collapse star was already repaired
      // this pass (keeps the per-pass node_to_elems / boundary_nodes maps valid for this collapse)
      std::set<dof_id_type> star_nodes;
      for (const auto & list : {best_deleted, best_reshaped})
        for (const auto eid : list)
        {
          const Elem * e = mesh->query_elem_ptr(eid);
          for (const auto i : make_range(4u))
            star_nodes.insert(e->node_id(i));
        }
      bool overlaps = false;
      for (const auto n : star_nodes)
        if (touched_nodes.count(n))
          overlaps = true;
      if (overlaps)
        continue; // retried next pass

      // Capture boundary face ids from all modified tets, keyed by their post-collapse face triple
      std::map<std::array<dof_id_type, 3>, std::set<boundary_id_type>> face_bcs;
      std::vector<boundary_id_type> ids;
      for (const auto & list : {best_deleted, best_reshaped})
        for (const auto eid : list)
        {
          Elem * e = mesh->query_elem_ptr(eid);
          for (const auto sd : make_range(e->n_sides()))
          {
            boundary_info.boundary_ids(e, cast_int<unsigned short>(sd), ids);
            if (ids.empty())
              continue;
            const auto ns = e->nodes_on_side(sd);
            auto sub = [&](unsigned int li)
            { return e->node_id(ns[li]) == best_G->id() ? best_K->id() : e->node_id(ns[li]); };
            auto & dst = face_bcs[faceKey(sub(0), sub(1), sub(2))];
            dst.insert(ids.begin(), ids.end());
          }
          boundary_info.remove(e);
        }
      // Transfer the gone node's nodeset ids to the kept node
      boundary_info.boundary_ids(best_G, ids);
      for (const auto bid : ids)
        boundary_info.add_node(best_K, bid);

      // Commit: reshape (G -> K), delete the collapsing tets (incl. the sliver)
      for (const auto eid : best_reshaped)
      {
        Elem * e = mesh->query_elem_ptr(eid);
        for (const auto i : make_range(4u))
          if (e->node_id(i) == best_G->id())
            e->set_node(i, best_K);
      }
      for (const auto eid : best_deleted)
        mesh->delete_elem(mesh->query_elem_ptr(eid));

      // Restore captured boundary ids onto the surviving (reshaped) faces
      for (const auto eid : best_reshaped)
      {
        Elem * e = mesh->query_elem_ptr(eid);
        for (const auto sd : make_range(e->n_sides()))
        {
          const auto ns = e->nodes_on_side(sd);
          auto it = face_bcs.find(faceKey(e->node_id(ns[0]), e->node_id(ns[1]), e->node_id(ns[2])));
          if (it != face_bcs.end())
            for (const auto bid : it->second)
              boundary_info.add_side(e, cast_int<unsigned short>(sd), bid);
        }
      }

      for (const auto n : star_nodes)
        touched_nodes.insert(n);
      ++num_repaired;
      repaired_in_pass = true;
    }
  }

  // Count any slivers that remain (no valid collapse was found for them)
  for (const auto & elem : mesh->active_element_ptr_range())
    if (elem->type() == TET4 && isTetSliver(*elem))
      ++num_skipped;

  if (num_repaired)
    mesh->prepare_for_use();
  if (num_repaired || num_skipped)
  {
    _console << "Number of tet sliver elements repaired by edge collapse: " << num_repaired
             << std::endl;
    if (num_skipped)
      _console << "Number of tet slivers that could not be collapsed (left in place): "
               << num_skipped << std::endl;
  }
}

void
MeshRepairGenerator::repairPyramidSlivers(std::unique_ptr<MeshBase> & mesh) const
{
  // Bounding-box volume scale, used by the volume-based sliver test
  const auto bbox = MeshTools::create_bounding_box(*mesh);
  const Point ext = bbox.max() - bbox.min();
  const Real vol_scale = std::max(std::abs(ext(0) * ext(1) * ext(2)), Real(1e-30));
  const Real vol_thresh = vol_scale * _sliver_volume_tol;

  // Sorted node-id key of a quad face (the 4-node analog of faceKey)
  auto quadKey = [](dof_id_type a, dof_id_type b, dof_id_type c, dof_id_type d)
  {
    std::array<dof_id_type, 4> f{a, b, c, d};
    std::sort(f.begin(), f.end());
    return f;
  };

  // A PYRAMID5 is a sliver if its volume is negligible or its apex is flat against its quad base
  auto isPyramidSliver = [&](const Elem & e)
  {
    if (e.type() != PYRAMID5)
      return false;
    if (_sliver_volume_tol > 0 && std::abs(e.volume()) < vol_thresh)
      return true;
    if (_sliver_flap_tol > 0)
    {
      const auto bn = e.nodes_on_side(4); // the quad base (4 vertices)
      const Point & apex = e.point(4);
      const Point & b0 = e.point(bn[0]);
      const Point & b1 = e.point(bn[1]);
      const Point & b2 = e.point(bn[2]);
      const Point & b3 = e.point(bn[3]);
      const Real area =
          0.5 * (b1 - b0).cross(b2 - b0).norm() + 0.5 * (b2 - b0).cross(b3 - b0).norm();
      if (area > 0)
      {
        const Real d = std::sqrt(std::min(geom_utils::pointTriangleDistanceSq(apex, b0, b1, b2),
                                          geom_utils::pointTriangleDistanceSq(apex, b0, b2, b3)));
        if (d < _sliver_flap_tol * std::sqrt(area))
          return true;
      }
    }
    return false;
  };

  std::size_t num_repaired = 0;
  std::size_t num_skipped = 0;

  bool repaired_in_pass = true;
  while (repaired_in_pass)
  {
    repaired_in_pass = false;

    // quad-face key -> ids of the 3D elements using that quad face (hex/prism/pyramid/polyhedron),
    // and the current PYRAMID5 slivers
    std::map<std::array<dof_id_type, 4>, std::vector<dof_id_type>> quad_to_elems;
    std::vector<dof_id_type> sliver_ids;
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->dim() != 3)
        continue;
      for (const auto s : make_range(elem->n_sides()))
      {
        const auto ns = elem->nodes_on_side(s);
        if (ns.size() == 4)
          quad_to_elems[quadKey(elem->node_id(ns[0]),
                                elem->node_id(ns[1]),
                                elem->node_id(ns[2]),
                                elem->node_id(ns[3]))]
              .push_back(elem->id());
      }
      if (isPyramidSliver(*elem))
        sliver_ids.push_back(elem->id());
    }

    BoundaryInfo & boundary_info = mesh->get_boundary_info();
    std::unordered_set<dof_id_type> touched_nodes;
    auto touches_repaired = [&touched_nodes](const Elem & e)
    {
      for (const auto i : make_range(e.n_nodes()))
        if (touched_nodes.count(e.node_id(i)))
          return true;
      return false;
    };

    for (const auto sid : sliver_ids)
    {
      Elem * p = mesh->query_elem_ptr(sid);
      if (!p || p->type() != PYRAMID5 || touches_repaired(*p) || !isPyramidSliver(*p))
        continue;

      // The quad base (side 4) and the element across it
      const auto pbase = p->nodes_on_side(4);
      const auto base_key = quadKey(
          p->node_id(pbase[0]), p->node_id(pbase[1]), p->node_id(pbase[2]), p->node_id(pbase[3]));
      Elem * nbr = nullptr;
      for (const auto nid : libmesh_map_find(quad_to_elems, base_key))
      {
        if (nid == sid)
          continue;
        Elem * cand = mesh->query_elem_ptr(nid);
        if (cand && cand->dim() == 3)
        {
          nbr = cand;
          break;
        }
      }
      // No element across the quad base (free boundary face), or the neighbor is busy this pass
      if (!nbr || touches_repaired(*nbr))
        continue;

      // Guard: the four triangular cap faces (apex + two adjacent base nodes) must be
      // non-degenerate, otherwise the union polyhedron would have a flat face (apex on a base edge)
      bool degenerate_cap = false;
      for (const auto s : make_range(4u))
      {
        const auto ns = p->nodes_on_side(s);
        if ((p->point(ns[1]) - p->point(ns[0])).cross(p->point(ns[2]) - p->point(ns[0])).norm() <
            1e-30)
          degenerate_cap = true;
      }
      if (degenerate_cap)
        continue;

      // Cheap early-out: the apex must project inside the shared quad base, otherwise the four cap
      // triangles cannot form a valid lid over the dissolved base and the union is non-convex. This
      // is necessary but not sufficient (it does not constrain the neighbor's geometry); the union
      // is fully validated below at construction time. (A flat pyramid whose apex projects outside
      // its base is caught by the volume test but cannot be absorbed.)
      {
        const Point & apex = p->point(4);
        const Point bv[4] = {
            p->point(pbase[0]), p->point(pbase[1]), p->point(pbase[2]), p->point(pbase[3])};
        const Point n = (bv[2] - bv[0]).cross(bv[3] - bv[1]); // quad normal (consistent winding)
        const Real tol = -1e-8 * n.norm();
        bool apex_inside = true;
        for (const auto i : make_range(4u))
          if ((bv[(i + 1) % 4] - bv[i]).cross(apex - bv[i]) * n < tol)
            apex_inside = false;
        if (!apex_inside)
          continue;
      }

      // Build the union polyhedron: the neighbor's faces (except the shared quad base) plus the
      // sliver pyramid's four triangular side faces. Capture boundary ids per face along the way.
      std::map<std::vector<dof_id_type>, std::set<boundary_id_type>> face_bcs;
      std::vector<boundary_id_type> ids;
      std::vector<std::shared_ptr<libMesh::Polygon>> faces;
      auto add_face = [&](Elem * e, unsigned int s)
      {
        const auto ns = e->nodes_on_side(s);
        auto poly = std::make_shared<libMesh::C0Polygon>(ns.size());
        std::vector<dof_id_type> key;
        for (const auto i : index_range(ns))
        {
          poly->set_node(i, e->node_ptr(ns[i]));
          key.push_back(e->node_id(ns[i]));
        }
        faces.push_back(poly);
        boundary_info.boundary_ids(e, cast_int<unsigned short>(s), ids);
        if (!ids.empty())
        {
          std::sort(key.begin(), key.end());
          face_bcs[key].insert(ids.begin(), ids.end());
        }
      };
      for (const auto s : make_range(nbr->n_sides()))
      {
        const auto ns = nbr->nodes_on_side(s);
        if (ns.size() == 4 && quadKey(nbr->node_id(ns[0]),
                                      nbr->node_id(ns[1]),
                                      nbr->node_id(ns[2]),
                                      nbr->node_id(ns[3])) == base_key)
          continue; // the dissolved shared quad
        add_face(nbr, s);
      }
      for (const auto s : make_range(4u))
        add_face(p, s);

      // Capture edge boundary ids (edgesets) on both elements, keyed by sorted endpoint-node pair,
      // so they can be re-applied to the matching edges of the polyhedron (side ids alone, captured
      // above, would drop any edgeset living on the pyramid or the neighbor).
      std::map<std::pair<dof_id_type, dof_id_type>, std::set<boundary_id_type>> edge_bcs;
      std::vector<boundary_id_type> eids;
      auto capture_edges = [&](Elem * e)
      {
        for (const auto ed : make_range(e->n_edges()))
        {
          boundary_info.edge_boundary_ids(e, cast_int<unsigned short>(ed), eids);
          if (eids.empty())
            continue;
          const auto en = e->nodes_on_edge(ed);
          auto a = e->node_id(en[0]);
          auto b = e->node_id(en[1]);
          if (a > b)
            std::swap(a, b);
          edge_bcs[{a, b}].insert(eids.begin(), eids.end());
        }
      };
      capture_edges(nbr);
      capture_edges(p);

      // Construct the union polyhedron and accept it only if it is a sound, convex cell. The
      // C0Polyhedron constructor tetrahedralizes the faces and throws if the union is non-convex or
      // has a flat sub-tet (a case the local apex guard above cannot rule out, e.g. a sloping
      // neighbor face), so the construction is wrapped: any failure leaves the sliver in place. The
      // old elements still exist here, so declining restores the original state exactly.
      const subdomain_id_type sub = nbr->subdomain_id();
      std::unique_ptr<libMesh::Node> mid_node;
      std::unique_ptr<libMesh::C0Polyhedron> poly_elem;
      try
      {
        poly_elem = std::make_unique<libMesh::C0Polyhedron>(faces, mid_node);
      }
      catch (const std::exception &)
      {
        // Non-convex / non-tetrahedralizable union: leave the sliver in place. NOTE: in optimized
        // libMesh builds this reject path can leak the constructor's interior node (the
        // C0Polyhedron constructor is not exception-safe on its fallback tetrahedralization); valid
        // (convex) unions are unaffected. This should be fixed upstream in libMesh.
        continue;
      }
      poly_elem->subdomain_id() = sub;
      libMesh::Node * mid_ptr = mid_node.get();
      if (mid_node)
        mesh->add_node(std::move(mid_node));
      Elem * added = mesh->add_elem(std::move(poly_elem));
      // The mid-element node now has a valid id, so volume() is well defined. Reject a degenerate
      // (non-positive volume) union and, in optimized builds where the constructor does not assert
      // on non-convexity, a non-convex result (which would self-overlap). convex() is the real
      // geometric check; volume()>0 alone always holds for a successfully built polyhedron.
      bool valid = added->volume() > 0;
      if (valid)
      {
        auto * poly = dynamic_cast<libMesh::Polyhedron *>(added);
        try
        {
          valid = poly && poly->convex();
        }
        catch (const std::exception &)
        {
          valid = false;
        }
      }
      if (!valid)
      {
        mesh->delete_elem(added);
        if (mid_ptr)
          mesh->delete_node(mid_ptr);
        continue; // invalid/non-convex union: leave the sliver in place
      }

      // Move the boundary ids from the old elements onto the matching faces of the polyhedron
      boundary_info.remove(p);
      boundary_info.remove(nbr);

      // Restore boundary ids onto the matching faces of the new polyhedron
      for (const auto s : make_range(added->n_sides()))
      {
        const auto ns = added->nodes_on_side(s);
        std::vector<dof_id_type> key;
        for (const auto i : index_range(ns))
          key.push_back(added->node_id(ns[i]));
        std::sort(key.begin(), key.end());
        auto it = face_bcs.find(key);
        if (it != face_bcs.end())
          for (const auto bid : it->second)
            boundary_info.add_side(added, cast_int<unsigned short>(s), bid);
      }

      // Restore edge boundary ids onto the matching edges of the new polyhedron
      if (!edge_bcs.empty())
        for (const auto ed : make_range(added->n_edges()))
        {
          const auto en = added->nodes_on_edge(ed);
          auto a = added->node_id(en[0]);
          auto b = added->node_id(en[1]);
          if (a > b)
            std::swap(a, b);
          auto it = edge_bcs.find({a, b});
          if (it != edge_bcs.end())
            for (const auto bid : it->second)
              boundary_info.add_edge(added, cast_int<unsigned short>(ed), bid);
        }

      for (const auto i : make_range(p->n_nodes()))
        touched_nodes.insert(p->node_id(i));
      for (const auto i : make_range(nbr->n_nodes()))
        touched_nodes.insert(nbr->node_id(i));
      // If the neighbor we absorbed was itself a sliver pyramid (two slivers glued base to base),
      // it is removed too and would otherwise vanish from both counts, so account for it here.
      const bool nbr_was_sliver = nbr->type() == PYRAMID5 && isPyramidSliver(*nbr);
      mesh->delete_elem(p);
      mesh->delete_elem(nbr);
      num_repaired += nbr_was_sliver ? 2 : 1;
      repaired_in_pass = true;
    }
  }

  // Count any pyramid slivers that remain (no quad-base neighbor or no valid absorption)
  for (const auto & elem : mesh->active_element_ptr_range())
    if (elem->type() == PYRAMID5 && isPyramidSliver(*elem))
      ++num_skipped;

  if (num_repaired)
    mesh->prepare_for_use();
  if (num_repaired || num_skipped)
  {
    _console << "Number of pyramid sliver elements absorbed into a neighbor: " << num_repaired
             << std::endl;
    if (num_skipped)
      _console << "Number of pyramid slivers that could not be absorbed (left in place): "
               << num_skipped << std::endl;
  }
}
